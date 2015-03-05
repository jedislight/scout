# scout
AI agent intended to explore open problem spaces via image capture and input bindings to generic applications

Build: Windows only for now. Haven't tested outside of VS2013 community edition solution file on Windows 7 so that is all I can garuntee functionality with

Running the agent: 
Needs to run with elevated privilages to be able to send input to the OS. Current test case is running JNES with the super mario
bros. rom (but if you don't own that game please use a test case you do own). Key Bindings are intended to be WASD directional keys
and ZX for AB. Don't recomend Start and Select as spamming pause isn't very useful :) Also recomend starting the agent after pressing Start to
begin to circumvent it needed to navigate the menus which require the Start button. Also recomend using JNES's cheat menu to enable infinite
lives so the main menu is never returned to. Currnetly the image capture is pointing to the inner part of the screen avoiding the status bar
and the GUI in game to avoid those elements from contributing the unique state. If you use a different test case you will need to adjust
your observation data retrival accordingly for best results.
(Wow, that is a lot of special setup. Hopefully this becomes more configuration driven soon)
