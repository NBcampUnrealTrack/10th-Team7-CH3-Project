#pragma once
// Enum이 필요한지는 더 알아보고 전달해드리겠습니다. 있어도 구동에 문제없기 때문에 임시로 넣어 놨습니다.
// UMETA는 블루프린트나 Details 패널 드롭다운에 "샷건 장탄수" 로 표시해준다고 합니다.
#include "CoreMinimal.h"
#include "CosTypes.generated.h"

UENUM(BlueprintType)                                // 무기 종류
enum class EWeaponType : uint8 // 블루프린트에 노출하려면 unit8(1바이트 정수 0~255)를 붙이는게 필수라고 합니다. BlueprintType은 uint8만 지원함.
{
    None    UMETA(DisplayName = "None"),            // 선택 안됨
    Nail    UMETA(DisplayName = "Nail"),            // 대못
    Shotgun UMETA(DisplayName = "Shotgun"),         // 샷건
    MAX     UMETA(Hidden) // enum에서 MAX는 마지막에 달아주는 것 같습니다. 더 알아보고 내용을 업데이트해드리겠습니다.
};

UENUM(BlueprintType)                                // 몬스터
enum class EEnemyType : uint8
{
    None        UMETA(DisplayName = "None"),        // 선택 안됨
    Ghoul       UMETA(DisplayName = "Ghoul"),       // 구울
    GhoulLord  UMETA(DisplayName = "GhoulLord"),    // 강화구울
    Gargoyle    UMETA(DisplayName = "Gargoyle"),    // 가고일
    Crow        UMETA(DisplayName = "Crow"),        // 까마귀..였던 것
    Boss        UMETA(DisplayName = "Boss"),        // 보스
    MAX         UMETA(Hidden)
};

UENUM(BlueprintType)                                    // 샷건 업그레이드
enum class EUpgradeType : uint8
{
    None          UMETA(DisplayName = "None"),          // 선택 안됨
    Damage        UMETA(DisplayName = "Damage"),        // 샷건 공격력
    FireRate      UMETA(DisplayName = "FireRate"),      // 샷건 공격 속도
    ReloadSpeed   UMETA(DisplayName = "ReloadSpeed"),   // 샷건 재장전 속도
    MaxAmmo       UMETA(DisplayName = "MaxAmmo"),       // 샷건 장탄수
    MAX           UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECosStatType : uint8     // StatType이라는 이름이 엔진에 있다네요. CosStatType으로 간다.         // 인챈트
{
    AllDamageAdd        UMETA(DisplayName = "AllDamageAdd"),        // 모든 무기 공격력 +n
    AllDamageMulti      UMETA(DisplayName = "AllDamageMulti"),      // 모든 무기 공격력 +n%

    ShotgunDamageAdd    UMETA(DisplayName = "ShotgunDamageAdd"),    // 샷건 공격력 +n
    ShotgunDamageMulti  UMETA(DisplayName = "ShotgunDamageMulti"),  // 샷건 공격력 +n%
    ShotgunReloadSpeed  UMETA(DisplayName = "ShotgunReloadSpeed"),  // 샷건 재장전 속도
    ShotgunMaxAmmo      UMETA(DisplayName = "ShotgunMaxAmmo"),      // 샷건 장탄수
    ShotgunFireRate     UMETA(DisplayName = "ShotgunFireRate"),     // 샷건 공격 속도

    MeleeDamageAdd      UMETA(DisplayName = "MeleeDamageAdd"),      // 대못 공격력 +n
    MeleeDamageMulti    UMETA(DisplayName = "MeleeDamageMulti"),    // 대못 공격력 +n%
    MeleeSpeedMulti     UMETA(DisplayName = "MeleeSpeedMulti"),     // 대못 공격 속도 +n%

    MaxHealth           UMETA(DisplayName = "MaxHealth"),           // 최대 체력
    MovementSpeed       UMETA(DisplayName = "MovementSpeed"),       // 이동 속도

    MAX                 UMETA(Hidden)                               
};