#include "GAS/Execution/AOExecCalc_Damage.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

void UAOExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	//Test Damage
	const float Damage = 10.0f;

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UAOAttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			-Damage
		)
	);
}
