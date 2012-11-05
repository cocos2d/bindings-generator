// first attempt at the debugger

var g = newGlobal("debug-global");
dbg = Debugger(g);

// fallback for no cc
cc = cc || {};
cc.log = log;

var breakpointHandler = {
	hit: function (frame) {
		var script = frame.script;
		_socketWrite(dbg.socket, "entering breakpoint: \n");
		_socketWrite(dbg.socket, "  " + script.url + ":" + script.getOffsetLine(frame.offset) + "\n");
		beginDebug(frame, frame.script);
	}
};

dbg.breakLine = 0;

var beginDebug = function (frame, script) {
	var stop = false;
	var offsets = [];
	// clear onStep from previous step (if any)
	if (frame) {
		if (dbg.breakLine > 0) {
			var curLine = script.getOffsetLine(frame.offset);
			if (curLine < dbg.breakLine)
				return;
		} else {
			dbg.breakLine = 0;
			frame.onStep = undefined;
		}
	}
	var processInput = function (str, socket) {
		var md = str.match(/^break current:(\d+)/);
		if (!frame && md) {
			var codeFound = false, i;
			offsets = script.getLineOffsets(parseInt(md[1], 10));
			_socketWrite(socket, "offsets: " + JSON.stringify(offsets) + "\n");
			for (i=0; i < offsets.lenth; i++) {
				script.setBreakpoint(offsets[i], breakpointHandler);
				codeFound = true;
			}
			if (offsets.length === 0) {
				var lines = script.getAllOffsets();
				var oldLine = parseInt(md[1], 10);
				for (var line = oldLine; line < lines.length; ++line) {
					if (lines[line]) {
						for (i=0; i < lines[line].length; i++) {
							script.setBreakpoint(lines[line][i], breakpointHandler);
							codeFound = true;
						}
						break;
					}
				}
			}
			if (!codeFound) {
				_socketWrite(socket, "invalid offset: " + offsets.join(",") + "\n");
			} else {
				_socketWrite(socket, "socket set at line " + md[1] + " for current file\n");
			}
			return;
		}
		md = str.match(/^b(reak)?\s+([^:]+):(\d+)/);
		if (md) {
			script = _getScript(md[2]);
			if (script) {
				cc.log("here");
				offsets = script.getLineOffsets(parseInt(md[3], 10));
				cc.log("offsets: " + offsets.join(","));
				// script.setBreakpoint(offsets[0], breakpointHandler);
				// _socketWrite(socket, "breakpoint set for line " + md[3] + " of script " + md[2] + "\n");
			} else {
				_socketWrite(socket, "no script with that name" + "\n");
			}
			return;
		}
		md = str.match(/^s(tep)?/);
		if (md && frame) {
			dbg.breakLine = script.getOffsetLine(frame.offset) + 1;
			frame.onStep = function () {
				beginDebug(frame, frame.script);
				return undefined;
			};
			stop = true;
			return;
		}
		md = str.match(/^c(ontinue)?/);
		if (md) {
			if (frame) {
				frame.onStep = undefined;
				dbg.breakLine = 0;
			}
			stop = true;
			return;
		}
		md = str.match(/^eval\s+(.+)/);
		if (md && frame) {
			var res = frame['eval'](md[1]),
				k;
			if (res['return']) {
				var r = res['return'];
				_socketWrite(socket, "* " + (typeof r) + "\n");
				if (typeof r != "object") {
					_socketWrite(socket, "~> " + r + "\n");
				} else {
					var props = r.getOwnPropertyNames();
					for (k in props) {
						var desc = r.getOwnPropertyDescriptor(props[k]);
						_socketWrite(socket, "~> " + props[k] + " = ");
						if (desc.value) {
							_socketWrite(socket, "" + desc.value);
						} else if (desc.get) {
							_socketWrite(socket, "" + desc.get());
						} else {
							_socketWrite(socket, "undefined (no value or getter)");
						}
						_socketWrite(socket, "\n");
					}
				}
			} else if (res['throw']) {
				_socketWrite(socket, "!! got exception: " + res['throw'].message + "\n");
			} else {
				_socketWrite(socket, "!!! invalid return for eval" + "\n");
				for (k in res) {
					_socketWrite(socket, "* " + k + ": " + res[k] + "\n");
				}
			}
			return;
		} else if (md) {
			_socketWrite(socket, "! no frame to eval in\n");
			return;
		}
		md = str.match(/^line/);
		if (md && frame) {
			_socketWrite(socket, "current line: " + script.getOffsetLine(frame.offset) + "\n");
			return;
		} else if (md) {
			_socketWrite(socket, "no line, probably entering script\n");
			return;
		}
		md = str.match(/^bt/);
		if (md && frame) {
			var cur = frame,
				stack = [cur.script.url + ":" + cur.script.getOffsetLine(cur.offset)];
			while ((cur = cur.older)) {
				stack.push(cur.script.url + ":" + cur.script.getOffsetLine(cur.offset));
			}
			_socketWrite(socket, stack.join("\n") + "\n");
			return;
		} else if (md) {
			_socketWrite(socket, "no valid frame\n");
			return;
		}
		_socketWrite("! invalid command" + "\n");
	};

	_socketOpen(1337, function (socket) {
		_socketWrite(socket, "> " + script.url + "\n");
		// set the client socket
		dbg.socket = socket;
		var str;
		while (!stop && (str = _socketRead(socket))) {
			processInput(str, socket);
			if (stop) {
				_socketWrite(socket, "<\n");
			}
		}
	});
};

dbg.onNewScript = function (script) {
	// skip if the url is this script
	var last = script.url.split("/").pop();
	if (last != "debugger.js" && script.url != "debugger eval code") {
		cc.log("on new script: " + script.url + "; will enter debugger");
		beginDebug(null, script);
	}
};

dbg.onDebuggerStatement = function (frame) {
	cc.log("!! on debugger");
	beginDebug(frame, frame.script);
};

dbg.onError = function (frame, report) {
	if (dbg.socket && report) {
		_socketWrite(dbg.socket, "!! exception @ " + report.file + ":" + report.line);
	}
	cc.log("!! exception");
	beginDebug(frame, frame.script);
};

function startDebugger(files, startFunc) {
	for (var i in files) {
		g['eval']("require('" + files[i] + "');");
	}
	if (startFunc) {
		g['eval'](startFunc);
	}
}
