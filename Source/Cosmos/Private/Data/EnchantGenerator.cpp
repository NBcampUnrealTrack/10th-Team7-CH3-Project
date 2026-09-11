#include "Data/EnchantGenerator.h"
#include "Data/EnchantData.h"
#include "Engine/DataTable.h"

// static -> 이 함수를 쓰려고 UEnchantGenerator 인스턴스를 따로 만들 필요가 없다.
// UEnchantGenerator::GenerateRandomEnchant(...)처럼 클래스 이름으로 바로 호출 가능

// UEnchantData* (리턴 타입) -> 이 함수가 최종적으로 완성된 인챈트 하나를 포인터로 돌려준다는 뜻

// UObject* Outer -> 새로 만든 인첸트 객체를 누구 소유로 둘 건지

// const UDataTable* StatPool, const UDataTable* SkillPool -> 데이터 테이블을 가져옴

// 인챈트를 만드는 함수
UEnchantData* UEnchantGenerator::GenerateRandomEnchant(UObject* Outer,
	const UDataTable* StatPool, const UDataTable* SkillPool)
{
	if (!StatPool || !SkillPool)
	{
		return nullptr;
	}

	// 리턴할 인챈트 (생성되는 인챈트)
	UEnchantData* NewEnchant = NewObject<UEnchantData>(Outer);

	// 인챈트 재료 (스탯, 스킬)
	const int32 EnchantMaterialCount = FMath::RandRange(2, 4);

	// 스킬 포함 여부 (어느 확률로 붙을지 미정이라 일단 false로 설정)
	const bool bIncludeSkill = false;
	NewEnchant->bHasSkill = bIncludeSkill;

	// 스킬을 포함할 경우
	// 스킬 데이터 테이블에서 해당 값들을 가져옴
	// 수치는 랜덤값이기 때문에 최소값과 최대값 사이의 랜덤값을 저장해줌
	if (bIncludeSkill)
	{
		// 스킬 데이터 테이블에 있는 정보를 다 가져와 AllSkills에 저장
		TArray<FEnchantSkillData*> AllSkills;
		SkillPool->GetAllRows(TEXT("GenerateRandomEnchant"), AllSkills);

		// AllSkills (스킬 데이터 테이블)이 비어있거나 하면 AllSkills.Num이 0이 됨
		// if문을 통해 0이 아닐 경우에만 스킬을 추가해준다. 
		if (AllSkills.Num() > 0)
		{
			// 뽑은 스킬을 정의해 준다.
			// 데이터 테이블에서 0번 인덱스 부터 num-1 인덱스 까지 중 무작위로 하나의 수를 뽑아 스킬을 저장한다
			const FEnchantSkillData* PickedSkill = AllSkills[FMath::RandRange(0, AllSkills.Num() - 1)];

			NewEnchant->SkillType = PickedSkill->SkillType;
			NewEnchant->SkillValue = FMath::FRandRange(PickedSkill->MinValue, PickedSkill->MaxValue);
			NewEnchant->SkillDuration = FMath::FRandRange(PickedSkill->MinDuration, PickedSkill->MaxDuration);
			NewEnchant->SkillCooldown = FMath::FRandRange(PickedSkill->MinCooldown, PickedSkill->MaxCooldown);
		}
	}
	
	// 나머지 인챈트 재료를 스탯으로 채움
	const int32 EnchantStatCount = EnchantMaterialCount - (bIncludeSkill ? 1 : 0);

	// 인챈트 스탯 데이터 테이블을 다 가져옴
	TArray<FEnchantStatData*> AllStats;
	StatPool->GetAllRows(TEXT("GenerateRondomEnchant"), AllStats);

	// 뽑을 스탯을 저장할 FEnchantStatData 형 배열을 만듦
	TArray<const FEnchantStatData*> Picked;

	// 스킬을 넣고 남은 자리에 인챈트 스탯을 넣음
	for (int32 i = 0; i < EnchantStatCount; i++)
	{
		// 예외처리: 
		if (AllStats.Num() == 0)
		{
			break;
		}

		// 스탯 데이터 테이블에 존재하는 스탯들 중 하나를 랜덤으로 가져옴
		const FEnchantStatData* PickedStat = AllStats[FMath::RandRange(0, AllStats.Num() - 1)];
		Picked.Add(PickedStat);

		AllStats.RemoveAll([PickedStat](const FEnchantStatData* AllStat)
			{
				if (AllStat == PickedStat) return true; // 같은 스탯 중복 방지

				const bool bPickedIsAll = (PickedStat->Category == EEnchantCategory::All);
				const bool bAllStatIsAll = (AllStat->Category == EEnchantCategory::All);
				const bool bPickedIsRangeOrMelee = (PickedStat->Category == EEnchantCategory::Range
					|| PickedStat->Category == EEnchantCategory::Melee);
				const bool bAllStatIsRangeOrMelee = (AllStat->Category == EEnchantCategory::Range
					|| AllStat->Category == EEnchantCategory::Melee);

				if (bPickedIsAll && bAllStatIsRangeOrMelee) return true;
				if (bPickedIsRangeOrMelee && bAllStatIsAll) return true;

				if (AllStat->Category == PickedStat->Category && AllStat->StatKind == PickedStat->StatKind)
				{
					const bool bOpposite =
						(PickedStat->ValueType == EEnchantValueType::Flat && AllStat->ValueType == EEnchantValueType::Percent) ||
						(PickedStat->ValueType == EEnchantValueType::Percent && AllStat->ValueType == EEnchantValueType::Flat);
					if (bOpposite) return true;
				}
				
				return false;
			});
	}

	// RolledStats 채우기
	for (const FEnchantStatData* Stat : Picked)
	{
		FEnchantRolledStat RolledStat;
		RolledStat.StatType = Stat->StatType;
		RolledStat.ValueType = Stat->ValueType;
		
		float RolledValue;
		if (Stat->ValueType == EEnchantValueType::Integer)
		{
			RolledValue = static_cast<float>(FMath::RandRange(FMath::RoundToInt(Stat->MinValue), FMath::RoundToInt(Stat->MaxValue)));
		}
		else
		{
			RolledValue = FMath::FRandRange(Stat->MinValue, Stat->MaxValue);
		}
		RolledStat.RolledValue = RolledValue;

		if (bIncludeSkill)
		{
			RolledStat.RolledValue *= 0.5f;
		}

		NewEnchant->RolledStats.Add(RolledStat);
	}

	return NewEnchant;
}
