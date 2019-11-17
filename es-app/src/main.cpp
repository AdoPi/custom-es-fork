//EmulationStation, a graphical front-end for ROM browsing. Created by Alec "Aloshi" Lofquist.
//http://www.aloshi.com

#include <EmulationStation.h>
#include <utils/sdl2/SyncronousEventService.h>
#include <Settings.h>
#include <utils/Log.h>
#include "MainRunner.h"

bool parseArgs(int argc, char* argv[], unsigned int* width, unsigned int* height)
{
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--resolution") == 0)
    {
      if (i >= argc - 2)
      {
        LOG(LogError) << "Invalid resolution supplied.";
        return false;
      }

      char* err;
      *width = (unsigned int)strtol(argv[i + 1], &err, 10);
      *height = (unsigned int)strtol(argv[i + 2], &err, 10);
      i += 2; // skip the argument value
    }
    else if (strcmp(argv[i], "--ignore-gamelist") == 0)
    {
      Settings::Instance().SetIgnoreGamelist(true);
    }
    else if (strcmp(argv[i], "--draw-framerate") == 0)
    {
      Settings::Instance().SetDrawFramerate(true);
    }
    else if (strcmp(argv[i], "--no-exit") == 0)
    {
      Settings::Instance().SetShowExit(false);
    }
    else if (strcmp(argv[i], "--debug") == 0)
    {
      Settings::Instance().SetDebug(true);
      Settings::Instance().SetHideConsole( false);
      Log::setReportingLevel(LogLevel::LogDebug);
    }
    else if (strcmp(argv[i], "--windowed") == 0)
    {
      Settings::Instance().SetWindowed(true);
    }
    else if (strcmp(argv[i], "--vsync") == 0)
    {
      bool vsync = (strcmp(argv[i + 1], "on") == 0 || strcmp(argv[i + 1], "1") == 0);
      Settings::Instance().SetVSync(vsync);
      i++; // skip vsync value
    }
    else if (strcmp(argv[i], "--max-vram") == 0)
    {
      char* err;
      int maxVRAM = (int)strtol(argv[i + 1], &err, 10);
      Settings::Instance().SetMaxVRAM(maxVRAM);
    }
    else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
    {
      #ifdef WIN32
      // This is a bit of a hack, but otherwise output will go to nowhere
      // when the application is compiled with the "WINDOWS" subsystem (which we usually are).
      // If you're an experienced Windows programmer and know how to do this
      // the right way, please submit a pull request!
      AttachConsole(ATTACH_PARENT_PROCESS);
      freopen("CONOUT$", "wb", stdout);
      #endif
      printf("EmulationStation, a graphical front-end for ROM browsing.\n"
             "Written by Alec \"Aloshi\" Lofquist.\n"
             "Version " PROGRAM_VERSION_STRING ", built " PROGRAM_BUILT_STRING "\n\n"
             "Command line arguments:\n"
             "--resolution [width] [height]	try and force a particular resolution\n"
             "--gamelist-only			skip automatic game search, only read from gamelist.xml\n"
             "--ignore-gamelist		ignore the gamelist (useful for troubleshooting)\n"
             "--draw-framerate		display the framerate\n"
             "--no-exit			don't show the exit option in the menu\n"
             "--hide-systemview		show only gamelist view, no system view\n"
             "--debug				more logging, show console on Windows\n"
             "--windowed			not fullscreen, should be used with --resolution\n"
             "--vsync [1/on or 0/off]		turn vsync on or off (default is on)\n"
             "--max-vram [size]		Max VRAM to use in Mb before swapping. 0 for unlimited\n"
             "--help, -h			summon a sentient, angry tuba\n\n"
             "More information available in README.md.\n");
      return false; //exit after printing help
    }
  }

  return true;
}

int main(int argc, char* argv[])
{
  // Get arguments
  unsigned int width = 0;
  unsigned int height = 0;
  if (!parseArgs(argc, argv, &width, &height))
    return 0;

  for(;;)
  {
    // Start the runner
    MainRunner runner(argv[0], width, height);
    MainRunner::ExitState exitState = runner.Run();

    LOG(LogInfo) << "EmulationStation cleanly shutting down.";
    switch(exitState)
    {
      case MainRunner::ExitState::Quit: return 0;
      case MainRunner::ExitState::FatalError: return 0;
      case MainRunner::ExitState::Relaunch: continue;
      case MainRunner::ExitState::NormalReboot:
      case MainRunner::ExitState::FastReboot:
      {
        LOG(LogInfo) << "Rebooting system";
        if (system("shutdown -r now") != 0)
          LOG(LogError) << "Error rebooting system";
        return 0;
      }
      case MainRunner::ExitState::Shutdown:
      case MainRunner::ExitState::FastShutdown:
      {
        LOG(LogInfo) << "Rebooting system";
        if (system("shutdown -h now") != 0)
          LOG(LogError) << "Error shutting system down";
        return 0;
      }
    }
  }
}

