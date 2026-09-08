// Fill out your copyright notice in the Description page of Project Settings.


#include "CosCharacter.h"

// Sets default values
ACosCharacter::ACosCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACosCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

