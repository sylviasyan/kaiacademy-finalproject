
libraries = -WinHttp.lib

.PHONY: dev

dev:
	export FLASK_APP=speedrun.app:app ;export FLASK_ENV=development;  flask run

implant: implant.cpp
	g++ -I\Users\sylviasy\Desktop\speedrun_FALL_2022\curl\include -L\Users\sylviasy\Desktop\speedrun_FALL_2022\curl\lib -std=c++11 implant.cpp -o implant.exe -lcurl

