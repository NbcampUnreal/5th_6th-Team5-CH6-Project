# Ward Zero

> 폐쇄된 병원을 탐색하며 기믹을 해결하고, 좀비와 보스를 돌파해 탈출을 목표로 하는 3인칭 서바이벌 호러 게임

---

## 프로젝트 소개

### 1. 기획 의도

중학생 시절 처음 접한 **바이오하자드 5**를 통해 좀비 액션 게임에 큰 흥미를 느끼게 되었습니다.  
이후 새로운 시리즈가 나올 때마다 직접 플레이하며, 공포와 액션이 결합된 게임이 주는 긴장감과 몰입감을 좋아하게 되었습니다.  

이러한 경험을 바탕으로, 저희는 플레이어가 어두운 분위기 속에서 전투와 탈출을 함께 경험할 수 있는 좀비 게임을 만들게 되었습니다.

저희가 만든 게임은 **어두운 공간 속에서 습격해오는 좀비와 맞서 싸우며 탈출을 진행하는 3인칭 공포 액션 게임**입니다.  
단순히 좀비를 처치하는 것에 그치지 않고, 제한된 시야와 긴박한 상황을 통해 공포감과 긴장감을 느낄 수 있도록 구성했습니다.  
또한 게임 진행 중에는 **다양한 기믹과 보스 요소를 배치하여 난이도를 조절하고, 플레이가 단조롭지 않도록 전개에 변화를 주는 방식**으로 설계했습니다.

---

### 2. 게임 소개

**Ward Zero**는 바이오하자드 특유의 긴장감과 탈출의 재미를 바탕으로,  
기믹 해결과 전투를 결합한 **3인칭 서바이벌 호러 게임**입니다.

플레이어는 폐쇄된 병원 공간을 탐색하며 아이템을 파밍하고,  
길을 막는 퍼즐과 기믹을 해결하면서 좀비와 보스를 처치해 탈출의 실마리를 찾아 나갑니다.

이 과정에서 제한된 자원과 전투 상황에 맞춰 생존 전략을 세워야 하며,  
최종적으로 모든 위협을 돌파하고 최종 보스를 처치했을 때 비로소 탈출에 성공할 수 있습니다.

---

## 핵심 특징

- **3인칭 서바이벌 호러**
  - 어두운 공간과 제한된 시야, 긴장감 있는 전투 중심 구성

- **기믹 + 전투 기반 진행**
  - 단순 전투만이 아닌 레버, 문, 카드키, 금고 등 다양한 상호작용 요소 배치

- **보스 중심의 단계적 게임 흐름**
  - 일반 탐색/전투 구간 → 중간 보스 → 최종 보스 → 탈출 구조

- **생존 전략 요소**
  - 한정된 자원, 탄약, 회복 아이템을 고려한 플레이 유도

- **서브시스템 기반 구조 설계**
  - UI, 저장, 문서, 상호작용 시스템을 분리해 관리성과 확장성 확보

---

## 게임 플레이

### 조작법

| 입력 키 | 동작 | 설명 |
| --- | --- | --- |
| `WASD` | 이동 | 캐릭터 이동 |
| `RMB` | 줌 | 총기 조준 |
| `LMB` | 공격 | 사격 |
| `Shift` | 스프린트 | 빠른 이동 |
| `E` | 상호작용 | 오브젝트 상호작용 |
| `Esc` | 메뉴 | 일시정지 / 설정 |
| `1` | 권총 장착 | 총기 전환 |
| `2` | SMG 장착 | 총기 전환 |
| `T` | 라이트 | 라이트 ON / OFF |

---

### 게임 흐름

```text
[Phase 1] 시작
    ↓
[Phase 2] 메인 게임플레이
    ↓
[Phase 3] 중간 보스
    ↓
[Phase 4] 최종 보스 / 탈출

기술 스택
분류	사용 기술
Engine	Unreal Engine 5.6
Procedural	SideFX Houdini Indie 20.5 / Houdini Engine 5.6
Language	C++ / VEX
Visual Scripting	Blueprint
IDE	Visual Studio 2022 / Rider
Version Control	Git / GitHub
Design	Figma / Houdini
Collaboration	Notion / Slack / Discord

주요 구현 내용
플레이어 시스템
컴포넌트 기반 캐릭터 구조
무기 장착 / 조준 / 사격 / 장전
반동 및 카메라 제어
상태 컴포넌트 기반 체력 및 전투 관리
Motion Warping 기반 상호작용 정렬

AI 전투 시스템
기본 좀비 State Machine
중간 보스 / 최종 보스 패턴 구현
Anim Notify 기반 타격 판정
소켓 기반 공격 트레이스
보스 전용 추적형 / 설치형 / 넉백형 스킬 구현
레벨 및 환경 시스템
환경 매니저 기반 구역 전환
조명 / BGM 전환
Chaos 기반 벽 파괴 연출
레벨 구역별 라이팅 제어 및 최적화
UI / 저장 시스템
LocalPlayerSubsystem 기반 UI 관리
무기 UI / 회복 UI / 상호작용 힌트 / 아이템 알림
Save / Load 시스템
문서 수집 및 뷰어 시스템
스테이지 진행도 저장 및 복원
상호작용 오브젝트
문, 슬라이딩 도어, 유리문
카드키
레버
탄약 박스
회복 아이템
금고 및 기타 인터랙션 오브젝트

아키텍처 포인트

주요 구성 요소
PlayerCombatComponent
PlayerCameraComponent
PlayerStatusComponent
SaveSubsystem
DocumentSubsystem
WeaponUISubsystem
InteractionHintSubsystem
ItemNotifySubsystem

개발 진행
전체 진행률
항목	진행률
전체 완성도	85%
코어 시스템	95%
콘텐츠	70%
폴리싱	80%
개발 기간
총 8주 진행
사전 기획 → 프로토타입 → 알파 → 베타 → 폴리싱 → QA/최적화 순서로 개발

사용 에셋 / 리소스
Realistic Lab. Laboratory Equipment
Interactive Safes and Lockers Collection
Hospital Props 02
Mobile Liquid Nitrogen Tank
DetectiveOffice
Server Battery 2 Slot
Vending Machine 2 LOWPOLY
Hospital environment (MODULAR)
RPG Creatures Voice Sound Effects
SCP Tentacles Monster
Reptilian monster
Undead Zombie Animation Bundle
Spy Girl Agent (Modular)

폴더 구조 예시
WardZero/
├─ Source/
├─ Content/
├─ Docs/
│  ├─ Images/
│  └─ README_assets/
├─ Config/
