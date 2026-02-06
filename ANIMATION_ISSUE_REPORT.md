# 🎮 애니메이션 블루프린트 깨짐 분석 보고서

## 📋 개요

PR 머지 이후 애니메이션 블루프린트(ABP)와 로코모션 BlendSpace(walk/run BS)가 깨진 문제에 대한 분석 보고서입니다.

---

## 🔍 분석 대상 PR

| PR | 제목 | 머지 대상 | 머지 날짜 | 변경 파일 수 |
|----|------|-----------|-----------|-------------|
| #20 | Fit. Animation Folder Move | Character_Prototype ← Character_Animation | 2026-02-05 | **105개** |
| #25 | Character animation | Character_Prototype ← Character_Animation | 2026-02-06 | 10개 |
| #26 | Character prototype | dev ← Character_Prototype | 2026-02-06 | 12개 |

---

## 🚨 핵심 문제점

### 1. 대규모 애니메이션 폴더 이동 (PR #20) — **가장 큰 원인**

PR #20에서 **모든 애니메이션 에셋**이 폴더 경로를 변경했습니다:

```
[이전 경로] Content/Animation/
[새 경로]   Content/Characters/Character_JC/Animation/
```

#### 이동된 주요 에셋 목록:

| 카테고리 | 이전 경로 | 새 경로 | 상태 |
|---------|-----------|---------|------|
| **ABP** | `Animation/ABP_Player.uasset` | `Characters/Character_JC/Animation/ABP_Player.uasset` | ⚠️ 삭제 후 재생성 |
| **BS_Walk** | `Animation/BlendSpace/BS_Walk.uasset` | `Characters/Character_JC/Animation/BlendSpace/BS_Walk.uasset` | ⚠️ 삭제 후 재생성 |
| **BS_Sprint** | `Animation/BlendSpace/BS_Sprint.uasset` | `Characters/Character_JC/Animation/BlendSpace/BS_Sprint.uasset` | ⚠️ 삭제 후 재생성 |
| **BS_Crouch** | `Animation/BlendSpace/BS_Crouch.uasset` | `Characters/Character_JC/Animation/BlendSpace/BS_Crouch.uasset` | ⚠️ 삭제 후 재생성 |
| Walk 애니메이션 (8개) | `Animation/Asset/Walk/` | `Characters/Character_JC/Animation/Asset/Walk/` | ⚠️ 삭제 후 재생성 |
| Run 애니메이션 (12개) | `Animation/Asset/Run/` | `Characters/Character_JC/Animation/Asset/Run/` | ⚠️ 삭제 후 재생성 |
| Idle 애니메이션 (5개) | `Animation/Asset/Idle/` | `Characters/Character_JC/Animation/Asset/Idle/` | ⚠️ 삭제 후 재생성 |
| Crouch 애니메이션 (12개) | `Animation/Asset/Crouch/` | `Characters/Character_JC/Animation/Asset/Crouch/` | ⚠️ 삭제 후 재생성 |
| IKRig (4개) | `Animation/IKRig/` | `Characters/Character_JC/Animation/IKRig/` | ⚠️ 삭제 후 재생성 |
| Retarget | `Animation/Retarget/` | `Characters/Character_JC/Animation/Retarget/` | ⚠️ 삭제 후 재생성 |
| 스켈레톤 | `Animation/IKRig/X_Bot_Skeleton.uasset` | `Characters/Character_JC/Animation/IKRig/X_Bot_Skeleton.uasset` | ⚠️ 삭제 후 재생성 |

**⚠️ 중요:** 모든 파일의 LFS OID(바이너리 해시)가 이전과 다릅니다. 이는 파일이 UE 에디터에서 다시 저장되었음을 의미하지만, **내부 참조 경로가 올바르게 업데이트되었는지 보장할 수 없습니다.**

#### 잠재적 깨짐 원인:
- UE 에디터의 "Move" 기능 대신 **파일 시스템에서 직접 이동**했을 가능성이 높음
- 이 경우 **Redirector가 생성되지 않아** 기존 참조가 깨짐
- `.uasset` 바이너리 내부의 경로 참조가 이전 경로(`/Game/Animation/...`)를 가리키고 있을 수 있음

---

### 2. 조깅 애니메이션 이름 변경 (PR #25) — **BlendSpace 깨짐의 직접적 원인**

PR #25에서 Run 애니메이션 에셋이 **삭제 후 새 이름으로 추가**되었습니다:

```
❌ 삭제됨: Characters/Character_JC/Animation/Asset/Run/Jogging__1_1.uasset
✅ 추가됨: Characters/Character_JC/Animation/Asset/Run/Jogging_2__UE.uasset
```

이것은 단순한 이름 변경이 아니라 **완전히 다른 에셋**입니다:
- `Jogging__1_1.uasset` (LFS OID: `ba6000...`, 729,717 bytes) — **삭제됨**
- `Jogging_2__UE.uasset` (LFS OID: `c181e2...`, 728,364 bytes) — **새로 추가됨**

**⚠️ 문제:** `BS_Walk.uasset`나 `BS_Sprint.uasset`가 내부적으로 `Jogging__1_1`을 참조하고 있었다면, 해당 참조가 깨집니다.

---

### 3. BlendSpace 수정 사항 (PR #25)

| 파일 | 이전 크기 | 새 크기 | 변화 | 분석 |
|------|-----------|---------|------|------|
| `BS_Walk.uasset` | 20,754 bytes | 23,418 bytes | +2,664 bytes | 새 애니메이션 참조 추가 (조깅 교체 관련으로 추정) |
| `BS_Sprint.uasset` | 8,684 bytes | 8,641 bytes | -43 bytes | 약간 줄어듦 (참조 업데이트) |
| `BS_Crouch.uasset` | - | - | **변경 없음** | ⚠️ PR #20에서 이동 후 업데이트되지 않았을 가능성 |

---

## 📁 수정/삭제된 애니메이션 관련 .uasset 파일 전체 목록

### PR #25 + #26에서 수정/삭제된 파일 (dev 브랜치에 영향)

| 상태 | 파일 경로 | 유형 |
|------|-----------|------|
| ✏️ 수정 | `Characters/Character_JC/Animation/ABP_Player.uasset` | Animation Blueprint |
| ✏️ 수정 | `Characters/Character_JC/Animation/BlendSpace/BS_Walk.uasset` | BlendSpace |
| ✏️ 수정 | `Characters/Character_JC/Animation/BlendSpace/BS_Sprint.uasset` | BlendSpace |
| ❌ 삭제 | `Characters/Character_JC/Animation/Asset/Run/Jogging__1_1.uasset` | Animation Sequence |
| ✅ 추가 | `Characters/Character_JC/Animation/Asset/Run/Jogging_2__UE.uasset` | Animation Sequence |
| ✅ 추가 | `Characters/Character_JC/Animation/Asset/Run/Turnning.uasset` | Animation Sequence |

### PR #20에서 삭제된 파일 (이전 경로)

| 파일 경로 | 유형 |
|-----------|------|
| `Animation/ABP_Player.uasset` | Animation Blueprint |
| `Animation/BlendSpace/BS_Walk.uasset` | BlendSpace |
| `Animation/BlendSpace/BS_Sprint.uasset` | BlendSpace |
| `Animation/BlendSpace/BS_Crouch.uasset` | BlendSpace |
| `Animation/Asset/Run/Jogging__1_.uasset` | Animation Sequence |
| `Animation/Asset/Run/Jogging__1_1.uasset` | Animation Sequence |
| `Animation/Asset/Run/ChangeDirection_UE.uasset` | Animation Sequence |
| `Animation/Asset/Run/ChangeDirection_UE_Montage.uasset` | AnimMontage |
| `Animation/Asset/Run/Fast_Run.uasset` | Animation Sequence |
| `Animation/Asset/Run/Injured_Run.uasset` | Animation Sequence |
| `Animation/Asset/Run/Running.uasset` | Animation Sequence |
| `Animation/Asset/Run/RunToStop.uasset` | Animation Sequence |
| `Animation/Asset/Run/RunningTurn180_1__UE.uasset` | Animation Sequence |
| `Animation/Asset/Run/Running_Turn_180.uasset` | Animation Sequence |
| `Animation/Asset/Run/Slow_Run.uasset` | Animation Sequence |
| `Animation/Asset/Run/Sprint.uasset` | Animation Sequence |
| `Animation/Asset/Walk/*.uasset` (8개) | Animation Sequence |
| `Animation/Asset/Idle/*.uasset` (5개) | Animation Sequence |
| `Animation/Asset/Crouch/*.uasset` (12개) | Animation Sequence |
| `Animation/IKRig/X_Bot_Skeleton.uasset` | Skeleton |
| `Animation/IKRig/X_Bot.uasset` | Skeletal Mesh |
| `Animation/IKRig/IK_*.uasset` (2개) | IK Rig |
| `Animation/Retarget/RTG_MixamoToQuinn.uasset` | IK Retargeter |

---

## 🔗 깨진 참조 분석

### 확인된 잠재적 깨진 참조:

1. **`/Game/Animation/` → 존재하지 않음**
   - PR #20에서 이 경로의 모든 파일이 삭제됨
   - 이전에 이 경로를 참조하던 모든 에셋이 깨질 수 있음
   - 영향 받는 에셋: ABP_Player, BS_Walk, BS_Sprint, BS_Crouch, 모든 BlendSpace 내부 참조

2. **`Jogging__1_1` → 존재하지 않음**
   - PR #25에서 삭제되고 `Jogging_2__UE`로 대체됨
   - BS_Walk 또는 BS_Sprint가 이 애니메이션을 참조하고 있었다면 깨짐

3. **스켈레톤/IKRig 참조**
   - `X_Bot_Skeleton.uasset`가 이동됨
   - 이 스켈레톤을 참조하는 모든 애니메이션 시퀀스의 내부 참조가 깨질 수 있음
   - 경로: `/Game/Animation/IKRig/X_Bot_Skeleton` → `/Game/Characters/Character_JC/Animation/IKRig/X_Bot_Skeleton`

4. **IK Retargeter 참조**
   - `RTG_MixamoToQuinn.uasset`가 이동됨
   - 소스/타겟 IK Rig 참조가 깨질 수 있음

---

## ⚔️ 머지 충돌 마커 분석

Git에서 `.uasset` 바이너리 파일은 일반적인 텍스트 머지가 불가능합니다. 분석한 PR들에서:

- **PR #26** (Character_Prototype → dev): 머지 커밋 `c84e528`에서 12개 파일이 변경됨
- 모든 `.uasset` 파일이 Git LFS로 관리되고 있어 바이너리 머지 충돌이 발생했을 가능성은 낮음
- 하지만 LFS 환경에서도 동일 파일을 양쪽 브랜치에서 수정했다면 **한쪽의 변경사항이 완전히 덮어쓰기**됨

**⚠️ 주의:** `.uasset` 바이너리 파일에는 텍스트 형태의 머지 충돌 마커 (`<<<<<<<`, `=======`, `>>>>>>>`)가 삽입되지 않습니다. 대신 Git은 한쪽 버전을 선택하거나 머지를 거부합니다. LFS를 사용하는 경우 마지막으로 push된 버전이 사용됩니다.

---

## 🛠️ 수정 방법 가이드

### 즉시 조치 사항

#### 1단계: UE 에디터에서 참조 확인
```
1. 프로젝트를 Unreal Editor에서 열기
2. Content Browser에서 BS_Walk, BS_Sprint, BS_Crouch 더블클릭
3. 빨간색으로 표시된 깨진 참조 확인
4. ABP_Player를 열어 State Machine 노드의 BlendSpace 참조 확인
```

#### 2단계: BlendSpace 참조 수정
```
1. BS_Walk.uasset 열기
2. 깨진 애니메이션 슬롯 확인 (Jogging__1_1 → Jogging_2__UE로 교체 필요)
3. 모든 애니메이션 참조가 새 경로를 가리키는지 확인:
   /Game/Characters/Character_JC/Animation/Asset/Walk/...
   /Game/Characters/Character_JC/Animation/Asset/Run/...
4. BS_Sprint, BS_Crouch도 동일하게 확인
```

#### 3단계: Redirector 정리
```
1. Content Browser > 우클릭 > Fix Up Redirectors in Folder
2. Content/Animation/ 폴더 (존재한다면)의 redirector 정리
3. Content/Characters/Character_JC/Animation/ 폴더의 redirector 정리
```

#### 4단계: ABP_Player 수정
```
1. ABP_Player 열기
2. Locomotion State Machine의 BlendSpace 노드 확인
3. BS_Walk, BS_Sprint 참조가 올바른 경로인지 확인:
   /Game/Characters/Character_JC/Animation/BlendSpace/BS_Walk
   /Game/Characters/Character_JC/Animation/BlendSpace/BS_Sprint
4. 깨진 노드가 있으면 새 경로로 재연결
```

#### 5단계: 전체 참조 검증
```
콘솔 명령어 (UE Editor):
  Asset Audit > Size Map 에서 깨진 참조 확인
  또는 Python Editor Script:
  
  import unreal
  asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
  # 깨진 참조가 있는 에셋 검색
```

---

## 📊 요약

| 항목 | 상태 | 설명 |
|------|------|------|
| 수정/삭제된 애니메이션 .uasset | ⚠️ | PR #20에서 47개 삭제, 47개 새로 추가 (폴더 이동). PR #25에서 1개 삭제, 2개 추가, 3개 수정 |
| 깨진 참조 가능성 | 🔴 높음 | 폴더 이동 시 내부 참조 경로가 업데이트되지 않았을 가능성 높음 |
| 스켈레톤/애니메이션 이동/이름변경 | 🔴 | `X_Bot_Skeleton`, `Jogging__1_1` → `Jogging_2__UE` |
| 머지 충돌 마커 | ✅ | 바이너리 LFS 파일이므로 텍스트 충돌 마커 없음. 단, 덮어쓰기 가능성 존재 |
| BS_Crouch 업데이트 누락 | ⚠️ | PR #20 이후 한 번도 수정되지 않음 — 이전 참조 경로 잔존 가능 |

### 가장 가능성 높은 원인 순위:
1. 🥇 **PR #20 폴더 이동**: Git에서 직접 이동하여 UE Redirector 미생성 → 내부 참조 경로 깨짐
2. 🥈 **PR #25 `Jogging__1_1` 삭제**: BlendSpace가 삭제된 애니메이션을 참조할 수 있음
3. 🥉 **스켈레톤 경로 변경**: `X_Bot_Skeleton`의 경로 변경으로 애니메이션 시퀀스 참조 깨짐

---

## 🔌 플러그인 / 소켓 문제 분석

> **참고:** 애니메이션 리디렉션은 성공적으로 완료됨. 아래는 남은 플러그인/소켓 관련 문제점입니다.

### 문제 1: `BaseZombie.cpp` — 에디터 전용 헤더 include 가드 누락

**파일:** `Ward_Zero/Source/Ward_Zero/Private/MonsterAI/Entity/BaseZombie.cpp`

**현재 코드 (문제):**
```cpp
#include "UnrealEdGlobals.h"          // ← 에디터 전용 헤더!
...
#include "Editor/UnrealEdEngine.h"    // ← 에디터 전용 헤더!
```

이 두 헤더는 `UnrealEd` 모듈에 속해 있지만:
- `Ward_Zero.Build.cs`에 `"UnrealEd"` 모듈 종속성이 **선언되어 있지 않음**
- Ward_Zero 모듈은 `"Type": "Runtime"`으로 선언되어 있어 에디터 전용 헤더 포함이 부적절
- 쉬핑(Shipping) 빌드에서 `UnrealEd` 모듈이 존재하지 않아 **컴파일 에러 발생 가능**

**수정 방법:**
```cpp
// 변경 전:
#include "UnrealEdGlobals.h"
...
#include "Editor/UnrealEdEngine.h"

// 변경 후:
#if WITH_EDITOR
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#endif
```

---

### 문제 2: `Ward_Zero.Build.cs` — Slate/SlateCore 종속성 불일치

**파일:** `Ward_Zero/Source/Ward_Zero/Ward_Zero.Build.cs`

**현재 코드 (문제):**
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    ...
    "UMG",
    "Slate"       // ← Public에 Slate만 있고 SlateCore 누락
});

// 아래 주석은 남아있음
// Uncomment if you are using Slate UI
// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
```

문제:
- `"Slate"`가 `PublicDependencyModuleNames`에 있지만 `"SlateCore"`가 누락됨
- `Slate` 위젯(예: `SNew()`, `FSlateBrush` 등)을 사용할 때 `SlateCore`가 필요
- 주석에는 Private에 넣으라고 되어있지만 실제로는 Public에 있음

**수정 방법:**
```csharp
// 옵션 A: Public에 SlateCore 추가
PublicDependencyModuleNames.AddRange(new string[] {
    ...
    "UMG",
    "Slate",
    "SlateCore"
});

// 옵션 B: Slate를 Private으로 이동 (권장)
// PublicDependencyModuleNames에서 "Slate" 제거 후:
PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
```

---

### 문제 3: `LXR` 마켓플레이스 플러그인 — 설치 누락 가능성

**파일:** `Ward_Zero/Ward_Zero.uproject`

**현재 설정:**
```json
{
    "Name": "LXR",
    "Enabled": true,
    "MarketplaceURL": "com.epicgames.launcher://ue/marketplace/product/bfe03ff858fa4b8d9766d7e9113f3829"
}
```

PR #28에서 `LXR` (Light eXtended Rig) 마켓플레이스 플러그인이 추가됨.
이 플러그인은 **Epic Games 마켓플레이스에서 별도 설치가 필요**합니다.

문제:
- `"Enabled": true`로 설정되어 있어, 플러그인이 설치되지 않은 환경에서 프로젝트 로딩 실패
- 에러 메시지 예시: `"Plugin 'LXR' failed to load because module 'LXR' could not be found"`
- 프로젝트의 `Plugins/` 폴더에 LXR 소스가 포함되어 있지 않음 (엔진 레벨 설치 필요)

**수정 방법 (선택):**

A) **모든 팀원이 LXR 설치** (필요한 경우):
```
Epic Games Launcher > Marketplace > LXR 검색 > Install to Engine 5.6
```

B) **LXR이 필수가 아닌 경우 .uproject에서 제거:**
```json
// Ward_Zero.uproject에서 아래 블록 삭제:
{
    "Name": "LXR",
    "Enabled": true,
    "MarketplaceURL": "com.epicgames.launcher://ue/marketplace/product/bfe03ff858fa4b8d9766d7e9113f3829"
}
```

C) **임시로 비활성화:**
```json
{
    "Name": "LXR",
    "Enabled": false,
    "MarketplaceURL": "com.epicgames.launcher://ue/marketplace/product/bfe03ff858fa4b8d9766d7e9113f3829"
}
```

---

### 문제 4: `BP_Ward_ZeroGameMode` 중복 에셋

`Content/` 디렉토리에 동일 이름의 GameMode Blueprint가 두 곳에 존재:

| 경로 | 크기 | 용도 |
|------|------|------|
| `Content/GameMode/BP_Ward_ZeroGameMode.uasset` | 21,444 bytes | **새 에셋** (PR #25에서 추가) |
| `Content/Ward_Zero/GameMode/BP_Ward_ZeroGameMode.uasset` | 2,389 bytes | **리디렉터** (원본에서 축소됨) |

`DefaultEngine.ini`의 GameMode 설정은 여전히 ThirdPerson 템플릿을 가리킴:
```ini
GlobalDefaultGameMode=/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.BP_ThirdPersonGameMode_C
```

**수정 방법:**
1. UE 에디터에서 `Content/Ward_Zero/GameMode/` 폴더의 리디렉터 정리 (Fix Up Redirectors)
2. 필요시 `DefaultEngine.ini`의 `GlobalDefaultGameMode` 경로를 새 위치로 업데이트

---

## 🎯 우선순위별 수정 체크리스트

- [ ] **[긴급]** `BaseZombie.cpp`: 에디터 전용 `#include`에 `#if WITH_EDITOR` 가드 추가
- [ ] **[긴급]** `Ward_Zero.Build.cs`: `"SlateCore"` 종속성 추가
- [ ] **[확인 필요]** `LXR` 플러그인: 모든 팀원 설치 확인 또는 `.uproject`에서 비활성화
- [ ] **[권장]** GameMode 리디렉터 정리 및 DefaultEngine.ini 경로 업데이트
