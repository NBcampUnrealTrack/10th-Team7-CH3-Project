# UI 해상도 및 인트로 전환 점검 (2026-09-27)

## 실행 기준

- 기본 게임 실행: 1920×1080, 창 모드. 전체화면 전환 시 선호 모드는 창 전체화면이다.
- PIE 새 창: 1920×1080. 에디터 안에서 실행하는 뷰포트의 크기는 에디터 패널 크기에 따라 달라진다.
- UMG 기준: 1920×1080, `ScaleToFit`. 가로와 세로 중 더 작은 배율을 사용해 다른 화면 비율에서도 고정 크기 패널이 잘리지 않도록 한다.
- `Config/DefaultGameUserSettings.ini`, `Config/DefaultEditorPerProjectUserSettings.ini`, `Config/DefaultEngine.ini`에 기본값을 기록했다. 기존 사용자 설정은 기본값보다 우선하므로 현재 PC의 설정도 1080p로 맞췄다.

## WBP 점검 범위

`/Game/Cosmos/WBP`의 Widget Blueprint 18개를 점검하고 15개를 수정했다.

| WBP | 조치 |
|---|---|
| MainMenu | 전체 화면 배경 및 게임오버 그룹 앵커, 게임오버 제목/점수/버튼 폰트 |
| EscHUD | 전체 화면 배경, 버튼 폭 및 가독성 |
| WaveResult | 전체 화면 배경, 제목 폰트, 큰 재화 숫자를 위한 텍스트 폭 |
| Forge | 배경 확장, 좌우/하단 패널 앵커, 제목/재화/종료 버튼 위치 |
| CombatHUD | 화면 밖 Day 표시, 탄약/체력/포션/재화 위치, 피격 화면 및 배경 앵커 |
| HPBar | 부모 영역과 내부 이미지 크기 정리, 숫자 폰트 |
| Ammo, Soul | 숫자 폰트, 재화 배경과 정렬 |
| HitReactScreen | 화면 비율에 맞춘 전체 화면 효과 |
| EnchantGetText | 설명 영역 확대 및 자동 줄바꿈 |
| EnchantInventory | 하단 버튼 영역을 260에서 60으로 축소, 슬롯 간격 및 화살표 정렬 |
| NailStatus, ShotgunStatus | 작은 설명 글자 확대, 제목/본문 구분, 진행바 간격 |
| PotionUpgrade | 하단 패널 크기와 글자/버튼 위치 |
| ShotgunUpgradeButton | 강화 버튼/제목 가독성 |
| EnchantSlot, InventorySlot, EnchantEquip | 기존 80×80 슬롯 및 300×100 장착 영역 유지 |

제목은 기존 `Megadeth_Font`, 설명·숫자·버튼은 엔진의 `Roboto`를 사용한다. 한글 제목은 가독성을 위해 본문용 폰트를 유지했다. 새 외부 폰트는 가져오지 않았다.

## 인트로 뒤 화면 전환

맵의 노출 최소/최대값은 이미 동일하게 고정되어 있고 SkyLight는 실시간 캡처를 사용한다. 특정 조명이 늦게 켜지는 정확한 원인은 확정하지 않았다. 새 월드의 초기 렌더링이 드러나지 않도록 메뉴에서 진입할 때 카메라를 검게 가린 상태로 준비 시간을 주고 페이드인하도록 처리했다. 맵의 조명 강도는 수정하지 않았다.

`BP_CosPlayerController → Class Defaults → UI → Transition`에서 조절한다.

- `Gameplay Warmup Seconds`: 1.25초. 월드는 계속 렌더링하고 플레이어 입력은 차단한다.
- `Gameplay Fade In Seconds`: 0.35초. 준비 시간 후 HUD와 입력을 복구하면서 화면을 밝힌다.

인트로 안내는 `Skip: Press Any Key`로 변경했다. 기존 영상 종료/건너뛰기 경로를 유지한다.

## 검증

- Cosmos 및 CosmosEditor Win64 Development 빌드 성공.
- 변경한 WBP 컴파일 및 저장 완료, 미저장 패키지 없음.
- 별도 게임 창에서 실제 뷰포트 1920×1080 확인: HUD, 대장간, 큰 숫자를 넣은 결과창을 캡처하여 검수했다.
- 별도 1280×800 게임 창에서 영어 건너뛰기 안내, 진입 중 HUD 숨김, 전환 후 HUD/이동/시점 입력 복구, 대장간 배치를 확인했다.
- 최종 게임 및 PIE 기본 설정을 1920×1080으로 복구했다.
- 새 패키지를 쿠킹/배포하지는 않았다. 검증에는 새 네이티브 빌드와 에디터의 별도 게임 실행을 사용했다.

수정 전 WBP 백업, 점검 스크립트, 캡처 및 검증 결과는 `Saved/UIAudit/`에 있다. 이 폴더는 로컬 검증 자료이며 Git에 포함되지 않는다.
