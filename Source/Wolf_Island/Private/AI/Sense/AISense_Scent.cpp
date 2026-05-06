#include "AI/Sense/AISense_Scent.h"
#include "AI/Sense/AISenseConfig_Scent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "DrawDebugHelpers.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"
#endif

UAISense_Scent::UAISense_Scent()
{
    ScentEventMaxAge = 10.0f;

    //TODO: 얘네도 바인딩 해제 안해도 되나...
    OnNewListenerDelegate.BindUObject(this, &UAISense_Scent::OnNewListenerImpl);
    OnListenerRemovedDelegate.BindUObject(this, &UAISense_Scent::OnListenerRemovedImpl);
}

void UAISense_Scent::RegisterEvent(const FAIScentEvent& Event)
{
    UE_LOG(LogTemp, Warning, TEXT("Scent RegisterEvent called! Location: %s, Instigator: %s"),
        *Event.Location.ToString(),
        Event.Instigator != nullptr ? *Event.Instigator->GetName() : TEXT("None"));

    // ���� �̺�Ʈ(Instigator ����) ����
    for (int32 i = 0; i < RegisteredEvents.Num(); i++)
    {
        if (RegisteredEvents[i].Instigator == Event.Instigator)
        {
            RegisteredEvents[i].Location = Event.Location;
            RegisteredEvents[i].Intensity = Event.Intensity;
            RegisteredEvents[i].MaxRange = Event.MaxRange;
            RegisteredEvents[i].Age = 0.f;

            // A�� �ٽ�: ���ŵ� ������ �� ������ �ο�
            RegisteredEvents[i].Sequence = ++GlobalScentSequence;

            RequestImmediateUpdate();
            return;
        }
    }

    // �ű� �̺�Ʈ �߰�
    FAIScentEvent NewEvent = Event;
    NewEvent.Age = 0.f;
    NewEvent.Sequence = ++GlobalScentSequence;

    RegisteredEvents.Add(NewEvent);
    RequestImmediateUpdate();
}


void UAISense_Scent::ReportScentEvent(UObject* WorldContextObject, const FVector& Location, float Intensity, float MaxRange, AActor* Instigator)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
        return;

    UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(World);
    if (PerceptionSystem)
    {
        FAIScentEvent ScentEvent;
        ScentEvent.Location = Location;
        ScentEvent.Intensity = Intensity;
        ScentEvent.MaxRange = MaxRange;
        ScentEvent.Instigator = Instigator;
        ScentEvent.Age = 0.0f;

        PerceptionSystem->OnEvent(ScentEvent);
    }
}

float UAISense_Scent::Update()
{
    AIPerception::FListenerMap& ListenersMap = *GetListeners();
    UWorld* World = GetWorld();
    if (!World) return 0.5f;

    const float Now = World->GetTimeSeconds();
    const float SenseDelta = (LastSenseUpdateTime < 0.f) ? 0.f : (Now - LastSenseUpdateTime);
    LastSenseUpdateTime = Now;

    // Age 업데이트 및 만료된 이벤트 삭제
    for (int32 i = RegisteredEvents.Num() - 1; i >= 0; --i)
    {
        RegisteredEvents[i].Age += SenseDelta;
        if (RegisteredEvents[i].Age > ScentEventMaxAge)
        {
            RegisteredEvents.RemoveAt(i);
        }
    }

    for (auto& Elem : ListenersMap)
    {
        FPerceptionListener& Listener = Elem.Value;

        const FDigestedScentProperties* Prop = DigestedProperties.Find(Listener.GetListenerID());
        if (!Prop) continue;

        const AActor* ListenerBodyActor = Listener.GetBodyActor();
        if (!ListenerBodyActor) continue;

        const FVector ListenerLoc = ListenerBodyActor->GetActorLocation();
        float DetectionRadiusSq = FMath::Square(Prop->ScentDetectionRadius);

        for (const FAIScentEvent& Event : RegisteredEvents)
        {
            float DistSq = FVector::DistSquared(ListenerLoc, Event.Location);
            float MaxRangeSq = FMath::Square(Event.MaxRange);

            if (DistSq <= MaxRangeSq && DistSq <= DetectionRadiusSq)
            {
                // A��: (Listener, Instigator)�� ������ ó�� ������ Ȯ��
                TMap<TWeakObjectPtr<AActor>, int32>& LastMap =
                    LastProcessedSequenceByListener.FindOrAdd(Listener.GetListenerID());

                int32* LastSeq = LastMap.Find(Event.Instigator);

                const bool bIsNewReport = (!LastSeq || *LastSeq < Event.Sequence);
                if (!bIsNewReport)
                {
                    continue; // �� report�� �ƴϸ� stimulus Ǫ�� �� ��
                }

                if (FMath::IsNearlyZero(Event.MaxRange))
                {
                    continue; // MaxRange�� 0�̸� ó������ �ʰ� ��ŵ
                }

                // �� report�� ���� stimulus 1ȸ Ǫ��
                float Distance = FMath::Sqrt(DistSq);
                float StrengthMultiplier = 1.0f - (Distance / Event.MaxRange);
                float FinalStrength = Event.Intensity * StrengthMultiplier;

                Listener.RegisterStimulus(Event.Instigator.Get(),
                    FAIStimulus(*this, FinalStrength, Event.Location, ListenerLoc));

                // ó�� ������ ����
                LastMap.Add(Event.Instigator, Event.Sequence);
            }
        }
    }

    if (RegisteredEvents.Num() == 0 || ListenersMap.Num() == 0)
    {
        return 1.0f;
    }
    return 0.5f;
}

void UAISense_Scent::OnNewListenerImpl(const FPerceptionListener& NewListener)
{
    UE_LOG(LogTemp, Warning, TEXT("Scent: New listener added!"));

    check(NewListener.Listener.IsValid());

    if (!NewListener.Listener.IsValid())
    {
        return;
    }

    UAISenseConfig* Config = NewListener.Listener->GetSenseConfig(GetSenseID());
    const UAISenseConfig_Scent* ScentConfig = Cast<const UAISenseConfig_Scent>(Config);

    if (ScentConfig)
    {
        FDigestedScentProperties PropertyDigest(*ScentConfig);
        DigestedProperties.Add(NewListener.GetListenerID(), PropertyDigest);
    }

    RequestImmediateUpdate();
}

void UAISense_Scent::OnListenerRemovedImpl(const FPerceptionListener& UpdatedListener)
{
    UE_LOG(LogTemp, Warning, TEXT("Scent: Listener removed!"));
    DigestedProperties.Remove(UpdatedListener.GetListenerID());

    LastProcessedSequenceByListener.Remove(UpdatedListener.GetListenerID());
}
UAISense_Scent::FDigestedScentProperties::FDigestedScentProperties()
{
    ScentDetectionRadius = 1000.f;
    bDisplayDebugSphere = false;
}

UAISense_Scent::FDigestedScentProperties::FDigestedScentProperties(const UAISenseConfig_Scent& SenseConfig)
{
    ScentDetectionRadius = SenseConfig.ScentDetectionRadius;
}