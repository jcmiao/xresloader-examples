copy ..\common\xres-code-generator\template\common\cpp\* ..\server\cfgsrc

py ..\common\xres-code-generator\xrescode-gen.py -i ..\common\xres-code-generator\template -p ..\common\sample.pb -o ..\server\cfgsrc -g ..\common\xres-code-generator\template\config_manager.h.mako -g ..\common\xres-code-generator\template\config_manager.cpp.mako -g ..\common\xres-code-generator\template\config_easy_api.h.mako -g ..\common\xres-code-generator\template\config_easy_api.cpp.mako -l "H:..\common\xres-code-generator\template\config_set.h.mako" -l "S:..\common\xres-code-generator\template\config_set.cpp.mako" -t "server"

py xresconv-cli-1.3.1/xresconv-cli.py -g server sample.xml