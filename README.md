# appear by jetlag

4k intro

*FINAL VERSION*

party version took 12th place at [Revision 2018](https://2018.revision-party.net/)

![Screenshot](https://yolp.omgwtf.ru/jetlag_appear_final.jpg)

## Links
- Pouet: [https://www.pouet.net/prod.php?which=75750](https://www.pouet.net/prod.php?which=75750)
- Video capture: [https://youtu.be/3af_z8LQOe4](https://youtu.be/3af_z8LQOe4)

## Credits
music, direction: keen

code, shaders: provod

### Third-party tools and libraries used
* [Crinkler](http://crinkler.net) for linking+packing an executable from ~10..20kb to <4096 bytes.
* [Shader minifier](https://github.com/laurentlb/Shader_Minifier) to make GLSL shader sources compress better.
* [4klang](http://4klang.untergrund.net/) for getting decent music in about 2 kilobytes

## Building and exploring
NOTE: requires beefy GPU. Expect 10-30fps on NVidia GTX Titan (~2013) or 20-60fps on AMD Radeon Fury X.

### Tool
Sources contain the tool that this intro was made in. Tool is in fact a really minimalistic file-monitor-shader-reloader.

Tool uses [atto](https://github.com/w23/atto) for window creation/OpenGL init/user input. So do `git submodule update --init` first.

Hit `e` to export current version of shader as `main.shader.glsl` file. It requires manual editing to be usable for intro compilation, see comments inside the file.

Tool also supports wsad+mouselook camera, but you'll need to uncomment this line in shader (don't forget to comment it back when exporting): `//O = $(vec3 cam_pos) * 3.; D = -normalize($(vec3 cam_dir));`

Hit `p` to print current camera coordinates into console window.

#### On Windows
- Open `intro.sln`
- Run `dump_audio` project to generate `audio.raw` file. Beware that 4klang/track is kind of unstable and may crash. There's no known fix for that.
- Build `tool` project using `Debug` configuration
- Run `run_tool.bat`

#### On Linux
- Run `make`. It will automatically pregenerate `audio.raw` (takes about a minute) and then run the tool.

### Intro
There are a few options to make intro executable:
- Debug: windowed, complains about shader compilation, no music.
- Release: fullscreen, no video if shader compilation failed, music, larger than 4k (about 4200 bytes)
- ReleaseSlow: same as Release, but takes about 5 minutes to be compressed by crinkler. Barely fits within 4096 bytes.

All dependencies (e.g. GLSL minification) are handled automatically.

There are also separate C and assembly versions. They are functionally very similar, but asm compresses better.

#### On Windows
Select `yo29_asm` project and desired configuration and build it.

#### On Linux
- `make win32-fast` will build Release as `appear-fast.exe`
- `make win32-slow` will build ReleaseSlow as `appear-slow.exe`

NOTE that for some reason Windows and Linux will produce different exes with different sizes for same targets (up to 20 bytes difference was observed, without any stable preference). Reason for this is not known, but they both run on Windows just fine.

There are also `appear.sh` and `appear.dbg` Linux targets, but they crash due to 4klang instability.

### Making video capture
Perfect 720p60 video capture can be made on Linux by running `make capture`. It will take quite some time to generate and encode the resulting video (about 30 minutes on my machine, but YMMV).

## Greetings
- alcatraz
-	consciousness
-	conspiracy
-	ctrl-alt-test
-	darklite
-	elix
-	fairlight
-	farbrausch
-	fl1ne
-	fms_cat
-	frag
-	kalium cyanide group
-	logicoma
-	mercury
-	nonoil
-	orange
-	orbitaldecay
-	proxium
-	prismbeings
-	quite
-	sands
-	sensenstahl
-	still
-	systemk
-	t-rex
-	the nephelims
-	throb
-	titan
-	tomohiro
-	vaahtera
-	virtual illusions
-	youth uprising
-	future crew :)

## License
MIT for tool and intro code. All third-party dependencies have their own licenses, go check them out.
