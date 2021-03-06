#include "platform/platformLibrary.h"

// Windows Entry
#if defined _WIN32 || defined __CYGWIN__
#include <windows.h>

typedef int (*mainFunc)(int argc, const char **argv);

int main(int argc, const char **argv)
{
#ifdef TORQUE_DEBUG
   LIBRARY_HANDLE hGame = openLibrary("Torque6_DEBUG");
#else
   LIBRARY_HANDLE hGame = openLibrary("Torque6");
#endif

   mainFunc enter = (mainFunc)getLibraryFunc(hGame, "winmain");
   if ( enter != 0 )
      return enter(0, NULL);
   return 0;
}

// OSX Entry
#elif __APPLE__

#include <iostream>
#include <cstdio>

typedef int (*mainFunc)(int argc, const char **argv);

int main(int argc, const char **argv)
{
    
#ifdef TORQUE_DEBUG
    LIBRARY_HANDLE hGame = openLibrary("Torque6_DEBUG");
#else
    LIBRARY_HANDLE hGame = openLibrary("Torque6");
#endif
    
    if(hGame == NULL)
    {
        //printf("%s\n", dlerror());
        std::cout << "Failed to load libTorque6.dylib";
        return 0;
    }
    
    mainFunc enter = (mainFunc)getLibraryFunc(hGame, "osxmain");
    if(enter == NULL)
    {
        //printf("%s\n", dlerror());
        std::cout << "Failed to find osxmain.";
        return 0;
    } else {
        return enter(argc, argv);
    }
    
    return 0;
}

// Linux Entry
#else

#include <iostream>
#include <cstdio>

typedef int (*mainFunc)(int argc, const char **argv);

int main(int argc, const char **argv)
{

#ifdef TORQUE_DEBUG
   LIBRARY_HANDLE hGame = openLibrary("Torque6_DEBUG", "./");
#else
   LIBRARY_HANDLE hGame = openLibrary("Torque6", "./");
#endif

   if(hGame == NULL)
   {
      //printf("%s\n", dlerror());
      std::cout << "Failed to load libTorque6.so";
      return 0;
   }

   mainFunc enter = (mainFunc)getLibraryFunc(hGame, "unixmain");
   if(enter == NULL)
   {
      //printf("%s\n", dlerror());
      std::cout << "Failed to find unixmain.";
      return 0;
   } else {
      return enter(argc, argv);
   }

   return 0;
}

#endif
