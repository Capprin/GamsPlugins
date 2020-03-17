// Fill out your copyright notice in the Description page of Project Settings.

#include "GamsGameInstance.h"
#include "GamsAgentsLogs.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "madara/utility/Utility.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "MoviePlayer.h"
#include "EngineUtils.h"
#include "GamsControllerThread.h"
#include <stdlib.h>
#include "UnrealAgentPlatform.h"
#include "GamsAgentManager.h"
#include "GamsVehicle.h"
#include "MadaraUnrealUtility.h"
#include "madara/knowledge/containers/Double.h"
#include "madara/knowledge/containers/Integer.h"
#include "madara/knowledge/containers/String.h"
#include "madara/knowledge/containers/Vector.h"
#include <algorithm>	

#include "GenericPlatform/GenericPlatformProcess.h"

namespace knowledge = madara::knowledge;
namespace transport = madara::transport;
namespace threads = madara::threads;
namespace containers = knowledge::containers;

void UGamsGameInstance::Init()
{
  Super::Init();

  UE_LOG (LogGamsGameInstance, Log,
    TEXT ("Init: entering"));

  FString filename;
  //FString filecontents;

  gams_game_instance = this;

  //gams::loggers::global_logger->clear();
  //gams::loggers::global_logger->add_file("gams_log.txt");
  //gams::loggers::global_logger->set_level(gams::loggers::LOG_MAJOR);

  agent_factory_ = new UnrealAgentPlatformFactory();

  // add the dynamic unreal agent platform factory
  std::vector <std::string> aliases;
  aliases.resize(3);
  aliases[0] = "default_agent";
  aliases[1] = "unreal_agent";
  aliases[2] = "dynamic_agent";

  UE_LOG (LogGamsGameInstance, Log,
    TEXT ("Init: adding aliases for agent factory"));

  gams::platforms::global_platform_factory()->add(aliases, agent_factory_);

  FString sim_settings_file = FPaths::Combine(
    FPaths::ProjectContentDir(),
    TEXT("Scripts"), TEXT("sim_settings.mf"));

  FString transport_settings_file = FPaths::Combine(
    FPaths::ProjectContentDir(),
    TEXT("Scripts"), TEXT("transport_settings.mf"));

  UE_LOG(LogGamsGameInstance, Log,
    TEXT("Init: loading transport settings from file %s."),
    *transport_settings_file);

  transport_settings.load_text(TCHAR_TO_UTF8(*transport_settings_file));

  filecontents_.SetNum(1);

  if (FFileHelper::LoadFileToString(filecontents_[0], *sim_settings_file))
  {
    UE_LOG(LogGamsGameInstance, Log,
      TEXT("Init: loading sim settings from file %s."),
      *sim_settings_file);
    kb.evaluate(TCHAR_TO_UTF8(*filecontents_[0]));
  }
  else
  {
    UE_LOG(LogGamsGameInstance, Warning,
      TEXT("Init: failed to load sim settings file. %s did not exist."),
      *sim_settings_file);
  }

  // log the transport settings to help with debugging
  FString debug_type = madara::transport::types_to_string(
    transport_settings.type).c_str();
  FString hosts;
  for (auto host : transport_settings.hosts)
  {
    hosts += ", ";
    hosts += host.c_str();
  }
  UE_LOG(LogGamsGameInstance, Log,
    TEXT("Init: transport settings: type=%s, hosts=[%s]"),
    *debug_type, *hosts);

  kb.attach_transport("GamsPluginsGameInstance", transport_settings);

  agents_loaded.set_name("swarm.loaded", kb);
  agents_loaded = 0;

  //UGameplayStatics::OpenLevel(this, "Plains");

  // seed the current world
  gams_current_world = GetWorld();
  swarm_size.set_name("swarm.size", kb);
  containers::Integer platform_animate("platform.animate", kb);
  if (*swarm_size == 0)
  {
    // if no swarm.size is specified, initialize 100
    swarm_size = 100;
  }

  // note that the default behavior is to not animate platforms
  should_animate = platform_animate.is_true();

  UE_LOG (LogGamsGameInstance, Log,
    TEXT ("Init: resizing controller to %d agents"),
    (int)*swarm_size);

  // create 100 agents
  controller.resize((size_t)*swarm_size);

  containers::Vector karl_files("karl_files", kb);

  size_t buf_size(0);
  char buf[128];

  /**
   * because UE4 completely messes up the STL, we have to avoid all
   * instances of classes like std::string, vector, etc. where we
   * allocate memory from the lib and reference it from our plugins.
   * Consequently, we have to revert to the early days of C where
   * every programmer had to manually track their null characters in
   * strings, hunt their own food, build houses with mud and feces,
   * etc.
   **/
  madara::utility::to_c_str(kb.get("platform.type"), (char*)buf, 128);

  agent_factory_->platform_type = buf;

  filecontents_.SetNum(karl_files.size());

  if (karl_files.size() > 0)
  {
    for (size_t i = 0; i < karl_files.size(); ++i)
    {
      madara::utility::to_c_str(karl_files[i], (char*)buf, 128);

      //const FString prefix("Scripts");
      //FString path(buf);

      //if (path.StartsWith(prefix, ESearchCase::CaseSensitive))
      //{
      //  // if the karl file begins with Scripts, then we need
      //  // to reference the Contents/Scripts directory
      //  filename = FPaths::Combine(FPaths::ProjectContentDir(),
      //    *path);
      //}
      //else
      //{
      //  filename = path;
      //}

      filename = madara::utility::create_path("Scripts", buf);

      UE_LOG(LogGamsGameInstance, Log,
        TEXT("Init: reading karl init from file %s"),
        *filename);

      if (FFileHelper::LoadFileToString(filecontents_[i], *filename))
      {
        UE_LOG(LogGamsGameInstance, Log,
          TEXT("Init: evaluating %d byte karl logic on each platform"),
          (int32)filecontents_[i].Len());


        controller.evaluate(TCHAR_TO_UTF8(*filecontents_[i]));
      }
    }
  }

  // setup delegates for changing maps (happens automatically on game start)

  FCoreUObjectDelegates::PreLoadMap.AddUObject(
    this, &UGamsGameInstance::OnPreLoadMap);
  FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
    this, &UGamsGameInstance::OnPostLoadMap);

  UnrealAgentPlatform::load_platform_classes();

  // run GAMS multicontroller at 2hz, unless user overrides with controller.hz
  controller_hz = 2.0f;

  enable_collisions = kb.get("platform.collisions").is_true();
  collision_type = enable_collisions ?
    ECollisionEnabled::PhysicsOnly : ECollisionEnabled::NoCollision;

  if (kb.exists("controller.hz"))
  {
    // because of STL funkiness in UE4, we try to be safe with memory
    containers::Double temp_hz("controller.hz", kb);
    controller_hz = *temp_hz;
  }

  UE_LOG(LogGamsGameInstance, Log,
    TEXT("Init: starting GAMS controller at %f hz"), controller_hz);

  threader_.set_data_plane(kb);
  threader_.run(controller_hz, "controller",
    new GamsControllerThread(controller), true);

  // check if level variables exists and if so, load the level
  if (kb.exists("level"))
  {
    madara::utility::to_c_str(kb.get("level"), (char*)buf, 128);
    
    UE_LOG(LogGamsGameInstance, Log,
      TEXT("Init: opening map %s"), *FString(buf));

    UGameplayStatics::OpenLevel(this, FName(buf));
  }
  else
  {
    // editor requires explicit call to change the world
#if UE_EDITOR
    OnPostLoadMap(gams_current_world);
#endif
  }
  UE_LOG (LogGamsGameInstance, Log,
    TEXT ("Init: leaving"));
}

float UGamsGameInstance::LoadingPercentage() const
{
  float result = (float)*agents_loaded;

  if (*swarm_size > 0)
  {
    result /= (float)*swarm_size;
  }
  else
  {
    result = 0;
  }

  return result;
}

void UGamsGameInstance::OnPreLoadMap(const FString& map_name)
{
  threader_.pause("controller");
  threader_.wait_for_paused("controller");

  if (manager_)
  {
    manager_->clear();

    controller.clear_knowledge();
    controller.refresh_vars();

    for (auto file : filecontents_)
    {
      controller.evaluate(TCHAR_TO_UTF8(*file));
    }
  }

  agents_loaded = 0;

  if (!GetTimerManager().IsTimerPaused(run_timer_handler_))
  {
    GetTimerManager().PauseTimer(run_timer_handler_);
  }

  if (!IsRunningDedicatedServer())
  {
    FLoadingScreenAttributes LoadingScreen;
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
    LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
    //WidgetBlueprint'/Game/Blueprints/LoadingScreen.LoadingScreen'

    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
  }
}

void UGamsGameInstance::OnPostLoadMap(UWorld* new_world)
{
  gams_current_world = new_world;

  UE_LOG(LogGamsGameInstance, Log,
    TEXT("post_level_load: "
      "initializing unreal_agent platforms"));

  UE_LOG(LogGamsGameInstance, Log,
    TEXT("post_level_load: creating args knowledge map"));

  std::string platform_prefix("platform.");

  // assign dynamic unreal platforms to the agents
  //madara::knowledge::KnowledgeMap args (kb.to_map_stripped(platform_prefix));
  //args["blueprint"] = "random";
  //args["blueprints.size"] = madara::knowledge::KnowledgeRecord::Integer(3);
  //args["blueprints.0"] = "/Game/Quadcopters/Blueprints/BP_Quadcopter_A.BP_Quadcopter_A";
  //args["blueprints.1"] = "/Game/Quadcopters/Blueprints/BP_Quadcopter_B.BP_Quadcopter_B";
  //args["blueprints.2"] = "/Game/Quadcopters/Blueprints/BP_Quadcopter_C.BP_Quadcopter_C";
  //args["location"] = "random";
  //args["orientation"] = "random";

  controller.init_platform("unreal_agent");

  FActorSpawnParameters spawn_parameters;
  spawn_parameters.SpawnCollisionHandlingOverride =
    ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  FTransform transform(
    FQuat(0.0f, 0.0f, 0.0f, 0.0f),
    FVector::ZeroVector);

  manager_ = gams_current_world->SpawnActor<AGamsAgentManager>(
        AGamsAgentManager::StaticClass(), transform, spawn_parameters);

  manager_->read(kb);

  //madara::knowledge::safe_clear(args);

  kb.send_modifieds();
  last_send_time_ = gams_current_world->UnpausedTimeSeconds;
  
  threader_.resume("controller");

  // defaults are 60hz game loop and a 5 second delay.
  float delta_time = gams_delta_time;
  float delay = 5.0f;

  // allow overrides for game hertz and delay
  if (kb.exists("game.hz"))
  {
    // because of STL funkiness in UE4, we try to be safe with memory
    containers::Double temp_hz("game.hz", kb);
    delta_time = (float)(*temp_hz / 1000.0f);
  }

  if (kb.exists("game.delay"))
  {
    // because of STL funkiness in UE4, we try to be safe with memory
    containers::Double temp_delay("game.delay", kb);
    delay = (float)(*temp_delay);
  }

  gams_delta_time = delta_time;

  UE_LOG(LogGamsGameInstance, Log,
    TEXT("Init: starting game loop with delta_time=%f after %fs"),
    delta_time, delay);

  GetTimerManager().SetTimer(run_timer_handler_, this,
    &UGamsGameInstance::GameRun, delta_time, true, delay);

  if (!GetTimerManager().IsTimerPaused(run_timer_handler_))
  {
    GetTimerManager().UnPauseTimer(run_timer_handler_);
  }

}

void UGamsGameInstance::Shutdown ()
{
  Super::Shutdown();

  threader_.terminate();
  threader_.wait();
  UnrealAgentPlatform::unload_platform_classes();
  TimerManager->ClearTimer(run_timer_handler_);
  delete agent_factory_;
  agent_factory_ = 0;
  manager_->clear();
  //delete manager_;
  //manager_ = 0;
}

void UGamsGameInstance::GameRun()
{
  UE_LOG(LogGamsGameInstance, Log,
    TEXT("controllerrun: calling"));

  UWorld * temp_world = GetWorld();

 // gams_delta_time = temp_world->DeltaTimeSeconds;

  if (gams_current_world != temp_world)
  {
    UE_LOG(LogGamsGameInstance, Log,
      TEXT("world has been changed"));

    gams_current_world = temp_world;
  }

  manager_->update(gams_delta_time);

/*
  for (TActorIterator<AGamsVehicle> actor_it(gams_current_world);
      actor_it; ++actor_it)
  {
    AGamsVehicle* actor = *actor_it;
    FVector dest;
    FVector location = actor->GetActorLocation();
    FVector diff;
    FVector next;

    madara::utility::to_vector_multiply(actor->dest, dest);

    diff = dest - location;
    madara::utility::calculate_delta(diff, next,
      actor->max_speed, temp_world->DeltaTimeSeconds);
    next += location;

    UE_LOG(LogGamsGameInstance, Log,
      TEXT("name=[%s], loc=[%s], dest=[%s], "
        "diff=[%s], next=[%s]"),
      *actor->agent_prefix,
      *location.ToString(), *dest.ToString(),
      *diff.ToString(), *next.ToString());

    actor->SetActorLocation(next, false, nullptr, ETeleportType::None);

    if (should_animate)
    {
      actor->animate(temp_world->DeltaTimeSeconds);
    }
  }*/

  if (temp_world->UnpausedTimeSeconds > last_send_time_ + 1.0f)
  {
    kb.send_modifieds();
    last_send_time_ = temp_world->UnpausedTimeSeconds;
  }
}

// global static variables
UWorld * gams_current_world(0);
float gams_delta_time(0.06);
UGamsGameInstance* gams_game_instance(0);