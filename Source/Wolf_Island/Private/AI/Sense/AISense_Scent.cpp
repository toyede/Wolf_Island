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
    // MaxRange가 0이면 어떤 리스너도 감지할 수 없으므로 등록 자체를 차단
    if (FMath::IsNearlyZero(Event.MaxRange))
    {
        return;
    }

    // 기존 이벤트(Instigator 동일) 갱신
    for (int32 i = 0; i < RegisteredEvents.Num(); i++)
    {
        if (RegisteredEvents[i].Instigator == Event.Instigator)
        {
            RegisteredEvents[i].Location  = Event.Location;
            RegisteredEvents[i].Intensity = Event.Intensity;
            RegisteredEvents[i].MaxRange  = Event.MaxRange;
            RegisteredEvents[i].Age       = 0.f;
            RegisteredEvents[i].Sequence  = ++GlobalScentSequence;

            RequestImmediateUpdate();
            return;
        }
    }

    // 신규 이벤트 추가
    FAIScentEvent NewEvent = Event;
    NewEvent.Age      = 0.f;
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

    if (RegisteredEvents.Num() == 0 || ListenersMap.Num() == 0)
    {
        return 1.0f;
    }

    for (auto& Elem : ListenersMap)
    {
        FPerceptionListener& Listener = Elem.Value;

        const FDigestedScentProperties* Prop = DigestedProperties.Find(Listener.GetListenerID());
        if (!Prop) continue;

        const AActor* ListenerBodyActor = Listener.GetBodyActor();
        if (!ListenerBodyActor) continue;

        const FVector ListenerLoc      = ListenerBodyActor->GetActorLocation();
        const float DetectionRadiusSq  = FMath::Square(Prop->ScentDetectionRadius);

        // FindOrAdd를 이벤트 루프 바깥으로 이동 → 이벤트마다 맵 탐색하던 비용 제거
        TMap<TWeakObjectPtr<AActor>, int32>& LastMap =
            LastProcessedSequenceByListener.FindOrAdd(Listener.GetListenerID());

        for (const FAIScentEvent& Event : RegisteredEvents)
        {
            // Instigator 유효성 조기 체크
            if (!Event.Instigator) continue;

            // MaxRange와 DetectionRadius 중 작은 값으로 단일 비교
            const float EffectiveRangeSq = FMath::Min(FMath::Square(Event.MaxRange), DetectionRadiusSq);
            const float DistSq = FVector::DistSquared(ListenerLoc, Event.Location);

            if (DistSq > EffectiveRangeSq) continue;

            int32* LastSeq = LastMap.Find(Event.Instigator);
            if (LastSeq && *LastSeq >= Event.Sequence) continue; // 이미 처리된 report

            // stimulus 발행
            const float Distance         = FMath::Sqrt(DistSq);
            const float StrengthMult     = 1.0f - (Distance / Event.MaxRange);
            const float FinalStrength    = Event.Intensity * StrengthMult;

            Listener.RegisterStimulus(Event.Instigator.Get(),
                FAIStimulus(*this, FinalStrength, Event.Location, ListenerLoc));

            LastMap.Add(Event.Instigator, Event.Sequence);
        }
    }

    return 0.5f;
}

void UAISense_Scent::OnNewListenerImpl(const FPerceptionListener& NewListener)
{
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