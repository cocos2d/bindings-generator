# About the Debugger

The debugger is a current experiment, allowing you to debug your javascript sessions either from the
simulator or from a device.

The basic idea is that if loaded, it will break javascript execution and open a TCP socket, where
you can use a telnet client to send and receive commands.

If you want, you can use the included one: dbg_client.rb, which has support for readline, meaning
history and auto-completion, as well a small inline help. This is a work in progress and is only
meant to be used as a simple inspector while we work our way to using firefox as the remote
debugger.

## Debugging

The first thing you need to do if you want to debug, is to add the debugger.js file to your project,
then run that script before your main entrypoint.

```c++
bool AppDelegate::applicationDidFinishLaunching()
{
	...
    ScriptingCore::getInstance()->runScript("debugger.js");
    ScriptingCore::getInstance()->runScript("main.js");
}
```

In your main javascript entry point, you need to add some code in order to setup the debug
environment:

```javascript
// main.js
if (typeof startDebugger !== "undefined") {
	// when debugger.js is loaded
	startDebugger(['hello.js'], 'startGame()');
} else {
	require('hello.js');
	startGame();
}
```

The idea is that if you loaded the debugger.js script, there will be a `startDebugger` function,
which you then need to call, with an array of all scripts that you will `require`. Most of the time
it's only one element, since it will be the main script. The second argument for `startDebugger` is
the main function that will actually start your game. If you don't have one, make sure you add it
so the debugger can start your code. This is also a good idea to avoid cluttering the global
namespace.

It is important that the main entry point is separated from your game code, and you should not pass
the entry point to `startDebugger`. If you keep it this way, then disabling the debugger is as
simple as commenting the `runScript("debugger.js")` line.

## A debugging session

Once you start your game with a debugger session to be attached, the debugger will stop execution
every time it loads a script, and it will also open a TCP socket on port 1337 (you can change that
port in the `debugger.js` script) that will remain listening until your app dies.

The commands supported by the debugger are the following:

* `break file.js:NN`: set a breakpoint on file.js at line NN. **CURRENTLY NOT WORKING PROPERLY**
* `step`: if you are in a breakpoint (or if you got into the debugger due to a `debugger` statement
  in your code), you can step into the next line.
* `continue`: resume execution.
* `eval <valid js code>`: will evaluate the passed string as javascript code, in the current
  execution frame. Note that with this command you can actually modify the content of variables if
  you wish. The debugger client will print the result of the evaluation, so you can also use this
  to inspect objects. If the result is not an object, it will return the string representation,
  otherwise, it will iterate over all the public keys of the object and print the result. The
  inspection is non-recursive.
* `line`: prints the line of code where the debugger is currently at (the line of the current
  execution offset).
* `bt`: prints the backtrace (how did we get here).

## Setting breakpoints

Currently breakpoints are not supported: you will be able to add a breakpoint, but due to an issue
with the current API, the breakpoint won't be added in the correct line.

A solution for this, is to set the breakpoints before execution, using the valid javascript keyword
`debugger`. This keyword will only have effect when there's a debugger attached, so it won't affect
your code if you don't run the debugger. The execution will be paused right after the debugger
keyword.

## Setting an automatic task to run the debugger

It might be useful for you to add a "start debugger client" task when you run your game on the
simulator from Xcode. Here are the necessary steps.

1. copy the `run_dbg_client.sh` and `dbg_client.rb` files to the root of your project (next to the
   `.xcodeproj`)
2. edit the debug scheme (Product -> Edit Scheme), and add the following pre-action

```sh
TERMINAL=/Applications/Utilities/Terminal.app

echo "will run debugger"
open -a ${TERMINAL} ${PROJECT_DIR}/dbg_client.rb
```

![Edit Scheme](https://raw.github.com/funkaster/cxx-generator/master/targets/spidermonkey/common/debugger1.png)

![Edit Scheme](https://raw.github.com/funkaster/cxx-generator/master/targets/spidermonkey/common/debugger2.png)

That's it. Now every time you run your game, the debugger client will open in a new terminal window.
