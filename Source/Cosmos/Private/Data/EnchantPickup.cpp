// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/EnchantPickup.h"

// Sets default values
AEnchantPickup::AEnchantPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnchantPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnchantPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

