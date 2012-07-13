var cc = cc || {}

cc.p = function (x, y) {
	var tmp = new Float32Array(2);
	tmp[0] = x;
	tmp[1] = y;
	return tmp;
}

var director = cc.Director.sharedDirector();
var audioEngine = cc.SimpleAudioEngine.sharedEngine();
var winSize = director.getWinSize();

log("size: " + winSize[0] + "," + winSize[1]);

var scene = cc.Scene.create();

for (var i=0; i < 100; i++) {
	var sprite = cc.Sprite.create("Icon.png");
	var pos = cc.p(Math.random() * 480, Math.random() * 320);
	sprite.setPosition(pos);
	scene.addChild(sprite);
}

audioEngine.setBackgroundMusicVolume(0.5);
audioEngine.playBackgroundMusic("bgmusic.mp3", true);

director.runWithScene(scene);
