# void
a simple void world generator
# commands
## voidreload
* reload settings.json
# settings.json
* path ./bds root directoriy/plugins/void/
```jsonc
{
  "allow_list": [],//can use regex unless use_regex is false
  "black_list": [],//can use regex unless use_regex is false
  "use_regex": false,
  "enable_overworld": true,//if true , enable void world generator in overworld
  "enable_nether": true,//if true , enable void world generator in nether
  "enable_the_end": true,if true , enable void world generator in the_end
  "reload_command_name": "voidreload",//the namw of voidreload command
  "enable_debug": false,//if true , use debug mode
  "list_type": "allow"//if value is "allow" , use allow_list . if value is "black" , use black_list
}
```
