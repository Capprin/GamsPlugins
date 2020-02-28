// Fill out your copyright notice in the Description page of Project Settings.

#include "GamsDjiPhantom.h"
//#include "UObject/ConstructorHelpers.h"
#include "ConstructorHelpers.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "GamsAgentsLogs.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AGamsDjiPhantom::AGamsDjiPhantom()
  : AGamsAerialVehicle()
{
  PrimaryActorTick.bCanEverTick = false;
  this->max_speed = 500.0f;

  mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
  //mesh->SetupAttachment(RootComponent);
  RootComponent = mesh;

  UE_LOG(LogUnrealAgentPlatform, Log,
    TEXT("DjiPhantom: constr: entering"));

  static ConstructorHelpers::FObjectFinder<UStaticMesh> mesh_asset(
    TEXT("/Game/Quadcopters/Mesh/SM_QuadcopterA_Main.SM_QuadcopterA_Main"));

  static ConstructorHelpers::FObjectFinder<UStaticMesh> rotor_asset(
    TEXT("/Game/Quadcopters/Mesh/SM_QuadcopterA_Rotor.SM_QuadcopterA_Rotor"));

  static ConstructorHelpers::FObjectFinder<UStaticMesh> cam_asset(
    TEXT("/Game/Quadcopters/Mesh/SM_QuadcopterA_Cam.SM_QuadcopterA_Cam"));

  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> mesh_mat(
    TEXT("/Game/Quadcopters/Materials/MI_QuadcopterA_01.MI_QuadcopterA_01"));

  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> rotor_mat(
    TEXT("/Game/Quadcopters/Materials/MI_QuadcopterA_02.MI_QuadcopterA_02"));

  if (mesh_asset.Succeeded())
  {
    UE_LOG(LogUnrealAgentPlatform, Log,
      TEXT("DjiPhantom: constr: loaded root mesh"));

    mesh->SetStaticMesh(mesh_asset.Object);
    mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -25.0f));
    mesh->SetWorldScale3D(FVector(1.0f));

    //movement = CreateDefaultSubobject<UMovementComponent>(TEXT("MovementComponent"));
    //movement->SetUpdatedComponent(mesh);
    //movement->InitialSpeed = 10.0f;
    //movement->MaxSpeed = 30.0f;
    //movement->bRotationFollowsVelocity = true;
    //movement->bShouldBounce = true;
    //movement->Bounciness = 0.3f;

    // setup the rotors
    if (rotor_asset.Succeeded())
    {
      UStaticMeshComponent* rotor1 =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rotor1"));
      UStaticMeshComponent* rotor2 =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rotor2"));
      UStaticMeshComponent* rotor3 =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rotor3"));
      UStaticMeshComponent* rotor4 =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rotor4"));

      rotor1->SetupAttachment(mesh);
      rotor2->SetupAttachment(mesh);
      rotor3->SetupAttachment(mesh);
      rotor4->SetupAttachment(mesh);

      rotor1->SetStaticMesh(rotor_asset.Object);
      rotor1->SetRelativeLocation(FVector(-12.106963f, 11.918432f, 16.028538f));
      rotor1->SetWorldScale3D(FVector(1.0f));

      rotor2->SetStaticMesh(rotor_asset.Object);
      rotor2->SetRelativeLocation(FVector(-12.096648f, -11.978424f, 16.028538f));
      rotor2->SetWorldScale3D(FVector(1.0f));

      rotor3->SetStaticMesh(rotor_asset.Object);
      rotor3->SetRelativeLocation(FVector(12.108811f, -11.961754f, 16.028538f));
      rotor3->SetWorldScale3D(FVector(1.0f));

      rotor4->SetStaticMesh(rotor_asset.Object);
      rotor4->SetRelativeLocation(FVector(12.097581f, 11.974735f, 16.028526f));
      rotor4->SetWorldScale3D(FVector(1.0f));

      if (rotor_mat.Succeeded())
      {
        rotor1->SetMaterial(0, rotor_mat.Object);
        rotor2->SetMaterial(0, rotor_mat.Object);
      }
      if (mesh_mat.Succeeded())
      {
        rotor3->SetMaterial(0, mesh_mat.Object);
        rotor3->SetMaterial(0, mesh_mat.Object);
      }

      //camera_boom = CreateDefaultSubobject<USpringArmComponent>(
      //  TEXT("CameraBoom"));
      //camera_boom->SetupAttachment(mesh);
      //camera_boom->TargetArmLength = 500.0f;
      //camera_boom->bUsePawnControlRotation = true;

      //camera->CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
      //camera->SetupAttachment(camera_boom, USpringArmComponent::SocketName);
      //camera->bUsePawnControlRotation = false;
    }
    else
    {
      UE_LOG(LogUnrealAgentPlatform, Warning,
        TEXT("DjiPhantom: constr: ERROR: couldn't load rotors"));
    }

    if (cam_asset.Succeeded())
    {
      UStaticMeshComponent* cam =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("cam"));
      cam->SetupAttachment(mesh);

      cam->SetStaticMesh(cam_asset.Object);
      cam->SetRelativeLocation(FVector(0.000001f, 0.0f, 7.260234f));
      cam->SetWorldScale3D(FVector(1.0f));
    }
    else
    {
      UE_LOG(LogUnrealAgentPlatform, Warning,
        TEXT("DjiPhantom: constr: ERROR: couldn't load camera"));
    }

    if (mesh_mat.Succeeded())
    {
      mesh->SetMaterial(0, mesh_mat.Object);
    }
    else
    {
      UE_LOG(LogUnrealAgentPlatform, Warning,
        TEXT("DjiPhantom: constr: ERROR: couldn't load root mesh material"));
    }


  } // end root mesh check
  else
  {
    UE_LOG(LogUnrealAgentPlatform, Warning,
      TEXT("DjiPhantom: constr: ERROR: couldn't load root mesh"));
  }
}

void AGamsDjiPhantom::BeginPlay()
{
}
