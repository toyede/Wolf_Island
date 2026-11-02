# 보스전 전용 모드 (졸업 전시용) — 맵 제작 단계별 가이드

기준: `KSH` 브랜치 / UE 5.5
결과물: `Content/KSH/Maps/BossArena.umap` + `Content/KSH/Game/GM_BossArena.uasset`

기존 보스 전용 동굴(`Cave_Level`)을 그대로 가져와서 쓰고, 준비실만 새로 만든다.
**싱글 / 멀티(최대 4인) 겸용 맵 하나**로 만든다. 코드 수정은 없다.

---

## STEP 1 — GameMode 만들기

1. 콘텐츠 브라우저에서 `Content/KSH/` 아래에 **`Game`** 폴더를 만든다.
2. `Content/JSY/Game/GameModes/BP_MultiGameMode` 를 우클릭 → **Create Child Blueprint Class**
   - `BP_MainGameMode` 가 아니라 **`BP_MultiGameMode`** 다.
3. 이름을 **`GM_BossArena`** 로 하고 `Content/KSH/Game/` 에 저장한다.
4. `GM_BossArena` 를 열고 **Class Defaults** 에서 한 개만 바꾼다.

| 속성 | 값 |
|---|---|
| `Turn On Auto Save` | **체크 해제** |

`Default Pawn Class` 는 건드리지 않는다. 멀티 게임모드는 `PlayerRoleClassList`(부모에 이미 채워져 있음)에서 역할별 폰을 뽑아 쓴다.

5. 같은 블루프린트의 **이벤트 그래프**에 역할 자동 배정을 넣는다. 이게 없으면 플레이어가 관전자 상태로 멈춘다.

   그래프 빈 곳 우클릭 → **`Event OnPostLogin`** 추가 → 아래 순서로 연결한다.

   ```
   Event OnPostLogin ─ New Player ─→ Get Player State
                                        │
                                        ▼
                                   Cast To MainPlayerState
                                        │ (As Main Player State)
                                        ▼
                                   Set Random Role
   ```

   - `Get Player State` 는 `New Player` 핀을 드래그해서 찾는다.
   - `Set Random Role` 은 `As Main Player State` 핀을 드래그해서 찾는다. (파라미터 없음)
   - 실행선: `Event OnPostLogin` → `Cast To MainPlayerState` → `Set Random Role`

6. 컴파일 → 저장 → 닫는다.

> 역할은 선장/요리사/정비사/군인 4개뿐이라 **최대 4인**이다. 5명 이상 접속시키면 안 된다.

---

## STEP 2 — 새 레벨 만들기

1. File → **New Level** → **Basic** 선택 (Open World 아님)
2. 아웃라이너에서 **`Floor`** 와 **`Cube`** 를 삭제한다. 라이팅 액터(DirectionalLight, SkyLight, SkyAtmosphere, Fog, VolumetricCloud)는 남겨둔다.
3. **File → Save Current Level As** → `Content/KSH/Maps/` 에 **`BossArena`** 로 저장
4. **Window → World Settings** → `GameMode Override` = **`GM_BossArena`**
5. 저장 (`Ctrl+S`)

---

## STEP 3 — 동굴 세트 가져오기

1. **File → Open Level** → `Content/Maps/Island_Single` 을 연다. (133MB라 로딩에 몇 분 걸린다)
2. 아웃라이너에서 **`Cave_Level`** 폴더를 찾는다.
3. `Cave_Level` 폴더를 클릭해서 선택한다. 하위 폴더 `Cave_Level/Portals` 까지 포함해 **21개** 액터가 선택돼야 한다.
   - 폴더 클릭으로 하위 액터가 안 잡히면, 폴더 화살표를 펼쳐서 맨 위 액터 클릭 → 맨 아래 액터 `Shift+클릭`
4. **`Ctrl+C`**
5. **File → Open Level** → `Content/KSH/Maps/BossArena`
   - **저장하겠냐고 물으면 "Don't Save".** Island_Single 은 절대 저장하지 않는다.
6. **Edit → Paste** (`Ctrl+V`). 원래 좌표 그대로 붙는다. "Paste Here" 는 쓰지 않는다.
7. 아웃라이너에 아래 21개가 들어왔는지 확인한다.

| 액터 | Location (X, Y, Z) | Yaw | Scale |
|---|---|---|---|
| `Cave` | `38045, 39963, 104` | `-80` | `0.3, 0.6, 0.4` |
| `ancient_tunnel` | `38234, 38462, 250` | `180` | `2.75, 1, 1` |
| `ancient_tunnel2` | `38233, 41577, 250` | `0` | `-2.75, 1, 1` |
| `Blocks_SimpleBlocksFlat_1_SM` | `38290, 40220, 1550` | `0` | `5, 60, 25` |
| `Blocks_SimpleBlocksFlat_1_SM2` | `38220, 43190, 540` | `-90` | `1, 15, 15` |
| (통로 입구 막음벽) | `38220, 36860, 540` | `-90` | `1, 15, 15` |
| `Blocks_WoodenBeamArch_3_SM` | `36880, 40020, 1660` | `-140` | `4, 4, 4.5` |
| `BP_TempleLight` ×9 | X `36700~39765` / Y `37912~42122` / Z `960` | — | `2, 2, 2` |
| `BP_BossSpawnPoint` | `38230, 40020, 600` | `90` | `1, 1, 1` |
| `BP_StatueSpawnPoint` | `38230, 40260, 750` | `-89` | `1, 1, 1` |
| `ToViliagePortal` | `38180, 37030, 260` | `90` | `1, 1, 1` |
| `BP_PortalPoint1` | `38200, 37270, 260` | `90` | `1, 1, 1` |

8. 이름을 두 개 바꾼다. (아웃라이너에서 `F2`)
   - `ToViliagePortal` → **`Portal_Exit`**
   - `BP_PortalPoint1` → **`PortalPoint_Cave`**
9. 저장

---

## STEP 4 — 석상 소환 지점 3개 추가

복사해 온 `BP_StatueSpawnPoint` 가 1개뿐이라 석상이 항상 같은 자리에 나온다. 3개 더 깐다.

1. 아웃라이너에서 `BP_StatueSpawnPoint` 선택 → `Ctrl+W` (Duplicate) 를 3번
2. 복제된 3개의 Location 을 아래로 바꾼다. Rotation 과 Z 는 건드리지 않는다.

| 액터 | Location (X, Y, Z) |
|---|---|
| `BP_StatueSpawnPoint2` | `37500, 40260, 750` |
| `BP_StatueSpawnPoint3` | `38960, 40260, 750` |
| `BP_StatueSpawnPoint4` | `38230, 39400, 750` |

3. 4개 모두 Details → **Actor → Tags** 에 **`StatueSpawn`** 이 들어 있는지 확인한다. (복제했으면 자동으로 따라온다)
4. 저장

> Z 는 `750` 을 유지한다. 보스가 스폰 지점 아래로 `-500` 까지만 라인트레이스해서 바닥에 붙이므로,
> 바닥(Z≈600)에서 500 이상 띄우면 석상이 공중에 뜬다.

---

## STEP 5 — 준비실 만들기

모두 `/Engine/BasicShapes/Cube` 를 끌어다 놓고 아래 값을 입력한다. 천장은 만들지 않는다.

| 이름 | Location (X, Y, Z) | Scale (X, Y, Z) |
|---|---|---|
| `Prep_Floor` | `38200, 34500, 150` | `20, 20, 1` |
| `Prep_Wall_N` | `38200, 35525, 500` | `21, 0.5, 6` |
| `Prep_Wall_S` | `38200, 33475, 500` | `21, 0.5, 6` |
| `Prep_Wall_E` | `39225, 34500, 500` | `0.5, 20, 6` |
| `Prep_Wall_W` | `37175, 34500, 500` | `0.5, 20, 6` |

바닥 윗면이 `Z = 200` 이 된다. 아래 STEP 6의 Z 값은 모두 이 기준이다.

필요하면 5개를 아웃라이너에서 선택 → 우클릭 → **Create Folder** 로 `Prep_Room` 폴더에 묶어둔다.

---

## STEP 6 — 준비실에 액터 배치

### 스폰 지점 (4인 기준)

`PlayerStart` 는 Place Actors 패널에서, `BP_BossEntryPlayerStart` 는 `/Game/JSY/Game/` 에서 끌어다 놓는다.
모두 Yaw `90`.

| 액터 | 이름 | Location (X, Y, Z) |
|---|---|---|
| `PlayerStart` | `PlayerStart1` | `37900, 33900, 300` |
| `PlayerStart` | `PlayerStart2` | `38100, 33900, 300` |
| `PlayerStart` | `PlayerStart3` | `38300, 33900, 300` |
| `PlayerStart` | `PlayerStart4` | `38500, 33900, 300` |
| `BP_BossEntryPlayerStart` | `BossEntry1` | `37900, 34100, 300` |
| `BP_BossEntryPlayerStart` | `BossEntry2` | `38100, 34100, 300` |
| `BP_BossEntryPlayerStart` | `BossEntry3` | `38300, 34100, 300` |
| `BP_BossEntryPlayerStart` | `BossEntry4` | `38500, 34100, 300` |

STEP 2에서 남겨둔 Basic 템플릿의 `PlayerStart` 가 있으면 그걸 `PlayerStart1` 로 쓰면 된다.

### 상자 · 아이템 · 포털

| 배치할 애셋 | 이름 | Location (X, Y, Z) | Yaw |
|---|---|---|---|
| `/Game/JSY/Interaction/Chest/BP_Chest` | `Chest_Supply` | `37700, 34500, 200` | `90` |
| `/Game/JSY/Interaction/BP_Pickup` | `Pickup_01` | `37700, 34750, 230` | `0` |
| `/Game/JSY/Interaction/BP_Pickup` | `Pickup_02` | `37850, 34750, 230` | `0` |
| `/Game/JSY/Interaction/BP_Pickup` | `Pickup_03` | `38000, 34750, 230` | `0` |
| `/Game/JSY/Interaction/BP_Pickup` | `Pickup_04` | `38150, 34750, 230` | `0` |
| `/Game/JSY/Interaction/BP_Pickup` | `Pickup_05` | `38300, 34750, 230` | `0` |
| `/Game/JSY/Interaction/BP_Pickup` | `Pickup_06` | `38450, 34750, 230` | `0` |
| `/Game/LJH/Portal/BP_Portals` | **`Portal_Entry`** | `38200, 35350, 200` | `270` |
| `/Game/LJH/Portal/BP_PortalPoint1` | **`PortalPoint_Prep`** | `38200, 35000, 220` | `270` |

저장.

---

## STEP 7 — 포털 연결

아웃라이너에서 액터를 **선택한 상태의 Details 패널**에서 입력한다. (블루프린트 에디터가 아니다)

### `Portal_Entry` (준비실)

| 속성 | 값 |
|---|---|
| `Portal ID` | `Arena_Entry` |
| `Target Portal` | `PortalPoint_Cave` |
| `Target Portal ID` | **비워둔다** |
| `Required Record ID` | `None` |
| `b Requires Boss Defeat` | **체크 해제** |
| `Boss Class To Spawn` | `BP_Boss` (`/Game/KSH/AI/BP_Boss`) |
| `Boss Spawn Point` | `BP_BossSpawnPoint` |
| `Exit Portal` | `Portal_Exit` |
| `b Use Random Offset` | **체크** |
| `Random Offset Radius` | `250` |
| `Spawn Z Offset` | `20` |
| `Interactable Data → Name` | `보스전 입장` |
| `Interaction Duration` | `1.0` |

### `Portal_Exit` (동굴)

| 속성 | 값 |
|---|---|
| `Portal ID` | `Arena_Exit` |
| `Target Portal` | `PortalPoint_Prep` |
| `Target Portal ID` | **비워둔다** |
| `Required Record ID` | `None` |
| `b Requires Boss Defeat` | **체크** |
| `Boss Class To Spawn` | **비워둔다 (None)** |
| `Boss Spawn Point` | **비워둔다 (None)** |
| `Exit Portal` | **비워둔다 (None)** |
| `b Use Random Offset` | **체크** |
| `Random Offset Radius` | `250` |
| `Interactable Data → Name` | `돌아가기` |
| `Interaction Duration` | `1.0` |

저장.

---

## STEP 8 — 멀티 준비 영역 설정

멀티에서는 **파티 전원이 포털의 준비 영역 안에 들어와야** 포털이 작동한다.
기본 박스가 작아서 그대로 두면 4인이 다 못 들어간다. 두 포털 다 키운다.

1. 아웃라이너에서 **`Portal_Entry`** 선택
2. Details 패널 위쪽 **컴포넌트 트리**에서 **`MultiReadyVolume`** 클릭
3. 아래 값을 입력한다.

| 속성 | 값 |
|---|---|
| Transform → `Location` (상대) | `400, 0, 150` |
| Shape → `Box Extent` | `400, 400, 250` |

4. 뷰포트에서 포털 **앞쪽** 바닥에 8m×8m 노란 박스가 깔렸는지 확인한다. 포털 뒤(벽 쪽)에 생겼으면 Location X 를 `-400` 으로 바꾼다.
5. **`Portal_Exit`** 에도 2~4번을 똑같이 반복한다.

저장.

> 전원이 안 들어오면 포털이 거부하면서 채팅창에 "모든 플레이어가 준비 영역에 있어야 포탈이 작동합니다" 가 뜬다.

---

## STEP 9 — 상자와 아이템 채우기

아이템마다 **한 칸에 쌓이는 최대 개수(`MaxAmount`)** 가 정해져 있다. 상자는 넘치는 만큼 칸을 더 쓰고,
바닥 아이템은 **`MaxAmount` 로 깎인다.** 아래 수량은 그걸 반영한 값이라 그대로 입력한다.

| ItemID | 이름 | 한 칸 최대 |
|---|---|---|
| `EQ004` | 돌 도끼 | 1 |
| `EQ005` | 돌 창 | 1 |
| `EQ006` | 횃불 | 1 |
| `FO102` | 치료제 | 1 |
| `FO009` | 생선구이 | 4 |
| `MA002` | 조약돌 | 16 |

### 상자

`Chest_Supply` 선택 → Details → **Init Setting → Init Items** 에 6줄 추가.

| ItemID | Amount | 차지하는 칸 |
|---|---|---|
| `EQ004` | `4` | 4 |
| `EQ005` | `2` | 2 |
| `MA002` | `48` | 3 |
| `FO102` | `6` | 6 |
| `FO009` | `8` | 2 |
| `EQ006` | `2` | 2 |

합계 **19칸**이다. `Chest Slots Size` 기본값이 `20` 이라 여기서 수량을 더 늘리면 넘치는 아이템이 사라진다.
더 넣고 싶으면 `Chest Slots Size` 를 같이 올린다. (`Chest Weight Capacity` 는 `2000` 이라 신경 쓸 필요 없다)

### 바닥 아이템

`Pickup_01` ~ `06` 각각 Details → **Item Data** 에서:

| 액터 | `Item Handle` → Data Table | `Item Handle` → Row Name | `Item Amount` | `Is Physics` |
|---|---|---|---|---|
| `Pickup_01` | `DT_ItemData` | `EQ004` | `1` | 체크 해제 |
| `Pickup_02` | `DT_ItemData` | `EQ005` | `1` | 체크 해제 |
| `Pickup_03` | `DT_ItemData` | `FO102` | `1` | 체크 해제 |
| `Pickup_04` | `DT_ItemData` | `FO102` | `1` | 체크 해제 |
| `Pickup_05` | `DT_ItemData` | `MA002` | `10` | 체크 해제 |
| `Pickup_06` | `DT_ItemData` | `FO009` | `2` | 체크 해제 |

`DT_ItemData` 경로는 `/Game/item/DT_ItemData` 다.

> 바닥 아이템은 아이템 데이터에 `Mesh` 가 있어야 보인다. `EQ006`(횃불) 은 `Mesh` 가 비어 있어서
> 바닥에 놓으면 **투명해진다.** 횃불은 상자에만 넣는다.

저장.

---

## STEP 10 — NavMesh 깔기

Place Actors 패널 → **Nav Mesh Bounds Volume** 을 2개 배치하고, Details → **Brush Settings** 에 크기를 입력한다.

| 이름 | Location (X, Y, Z) | Brush X / Y / Z |
|---|---|---|
| `Nav_Prep` | `38200, 34500, 500` | `2100 / 2100 / 800` |
| `Nav_Cave` | `38230, 40025, 900` | `3400 / 6600 / 1800` |

배치 후 뷰포트에서 **`P`** 를 눌러 초록색 NavMesh를 확인한다.
준비실 바닥과 동굴 바닥이 **빈틈없이** 초록색이어야 한다. 안 덮인 곳이 있으면 Brush 크기를 키운다.

저장.

---

## STEP 11 — 조명 정리

1. `DirectionalLight` 선택 → Rotation `Pitch -50, Yaw -30`, Intensity `3.0`
2. `DirectionalLight`, `SkyLight` 둘 다 Details → Transform → **Mobility = Movable**
3. `SkyLight` → **Real Time Capture** 체크
4. 뷰포트에서 준비실 안으로 들어가 밝기를 확인한다. 어두우면 `SkyLight` Intensity 를 `1.0 → 2.0` 으로 올린다.
5. 동굴은 복사해 온 `BP_TempleLight` 9개가 조명을 담당한다. 추가 작업 없다.

> `BP_DynamicSky_ver3` 는 배치하지 않는다.

저장.

---

## STEP 12 — 1인 테스트

툴바 **Play** 드롭다운 → `Number of Players` = `1`, `Net Mode` = **Play As Listen Server** → **Play**

- [ ] 준비실에서 스폰되고, 역할 캐릭터가 나온다 (관전자 상태로 멈추면 STEP 1의 5번 확인)
- [ ] 바닥 아이템 6개가 **보이고** 집을 수 있다 (투명하면 해당 아이템에 `Mesh` 가 없는 것)
- [ ] 상자를 열면 6종 19칸이 들어 있다
- [ ] 무기를 장착하고 공격이 나간다
- [ ] 포털 앞 준비 영역에 서면 `보스전 입장` 프롬프트가 뜨고, 1초 꾹 누르면 작동한다
- [ ] 동굴로 이동되고, 보스가 멀리서 걸어온다
- [ ] 보스 HP 바가 뜨고, 최대 HP가 `250` 이다
- [ ] 보스 기본 공격 3종이 나온다
- [ ] HP 50% 이하 → 경고 이펙트 후 석상이 소환된다 (4곳 중 랜덤)
- [ ] 석상이 **바닥에 붙어** 있다
- [ ] 석상을 안 부수면 보스 HP가 회복된다
- [ ] 늑대가 2마리쯤 소환된다
- [ ] 보스 돌진이 석상에 맞으면 그로기가 된다
- [ ] HP 30% 이하 → 3페이즈 패턴
- [ ] 보스 사망 → `Portal_Exit` 외형이 바뀌고 `돌아가기` 가 가능해진다
- [ ] `Portal_Exit` 로 준비실 복귀
- [ ] 사망 → 준비실에서 리스폰, 보스는 사라짐, 다시 입장 가능
- [ ] 동굴에서 걸어서 준비실로 나갈 수 없다
- [ ] 10분쯤 플레이 후 재시작 → 상자가 다시 가득 차 있다

---

## STEP 13 — 멀티 테스트

툴바 **Play** 드롭다운 → `Number of Players` = `4`, `Net Mode` = **Play As Listen Server** → **Play**

- [ ] 창 4개가 각각 다른 역할 캐릭터로 스폰된다 (같은 역할이 겹치면 안 된다)
- [ ] 4명 다 상자를 열 수 있다 (한 명이 열고 있는 동안 다른 사람은 못 여는 게 정상)
- [ ] **1명만** 준비 영역에 들어가서 포털을 쓰면 거부되고, 채팅창에 준비 영역 알림이 뜬다
- [ ] 4명 다 준비 영역에 들어가면 포털이 작동하고 **전원이 같이** 동굴로 이동한다
- [ ] 보스 최대 HP가 `1000` 이다 (4명 × 250)
- [ ] 클라이언트 창에서도 보스 애니메이션·공격·소환이 제대로 보인다
- [ ] 클라이언트 창에서도 보스 HP 바가 깎인다
- [ ] 1명이 죽으면 그 사람만 준비실에서 리스폰되고, 보스는 남은 사람을 계속 노린다
- [ ] 전원 사망 → 보스가 사라지고, 다시 입장할 수 있다
- [ ] 보스 사망 후 **클라이언트 창에서도** `Portal_Exit` 외형이 바뀌고 `돌아가기` 가 된다
- [ ] 4명 다 준비 영역에 모이면 전원이 같이 준비실로 복귀한다

### 패키징 후 실제 접속

- 호스트: `Wolf_Island.exe /Game/KSH/Maps/BossArena?listen`
- 클라이언트: 아무 맵으로 실행한 뒤 콘솔(`~`)에서 `open <호스트_IP>`

---

## 안 될 때

| 증상 / 로그 | 조치 |
|---|---|
| 캐릭터 없이 공중에서 관전만 됨 | STEP 1의 `Event OnPostLogin` → `Set Random Role` 연결 누락 |
| `Entered Players Role : 0` | 같음. 역할이 `NONE` 이면 폰이 안 나온다 |
| `[PORTAL] TARGET PORTAL IS NOT VALID` | `Target Portal` 미지정이거나 `Target Portal ID` 에 값이 들어감 → ID 칸을 비운다 |
| `[PORTAL] CAN'T INTERACT` | `Portal_Entry` 의 `b Requires Boss Defeat` 가 켜져 있다 → 끈다 |
| 다 모였는데도 준비 영역 알림이 계속 뜸 | STEP 8의 `Box Extent` 를 더 키운다. 관전자(역할 NONE) 플레이어가 한 명이라도 있으면 영원히 막힌다 |
| 포털을 타도 보스가 안 나옴 | `Portal_Entry` 의 `Boss Class To Spawn` 이 비어 있다 |
| `[GAMEMODE] Boss spawned but ExitPortal is invalid` | `Portal_Entry` 의 `Exit Portal` 미지정 |
| 보스를 잡아도 포털이 안 열림 | 같음. `Exit Portal` = `Portal_Exit` 확인 |
| `[Boss] Statue spawn discovery tag='StatueSpawn', found=0` | `BP_StatueSpawnPoint` 의 Actor Tags 에 `StatueSpawn` 이 없다 |
| 보스가 제자리에서 안 움직임 | `Nav_Cave` NavMesh 가 바닥을 안 덮음 → `P` 로 확인 후 Brush 키우기 |
| 늑대가 안 나옴 | 같음 (NavMesh) |
| 석상이 공중에 뜸 | 해당 `BP_StatueSpawnPoint` 의 Z 를 바닥 +150 이내로 내린다 |
| 상자가 비어 있음 | `GM_BossArena` 의 `Turn On Auto Save` 확인 후, `Saved/SaveGames/MTEST001_BossArena.sav` 삭제 |
| 5번째 접속자에서 크래시 | 역할이 4개뿐이다. 4인 초과로 받지 않는다 |
| 바닥 아이템 개수가 입력값보다 적다 | 그 아이템의 한 칸 최대치로 깎인 것이다. STEP 9의 표를 쓴다 |
| 바닥 아이템이 안 보임 | 아이템 데이터에 `Mesh` 가 없다. 다른 아이템으로 바꾼다 |
| 상자에 넣은 게 일부 없다 | 칸 수 합계가 `Chest Slots Size`(20)를 넘었다 |

---

## 커밋

새로 생기는 파일은 이 둘뿐이다.

```
Content/KSH/Maps/BossArena.umap
Content/KSH/Game/GM_BossArena.uasset
```

기존 `.umap` 은 하나도 수정하지 않는다. 커밋 전에 `git status` 로 `Content/Maps/` 아래가 안 바뀌었는지 확인한다.
