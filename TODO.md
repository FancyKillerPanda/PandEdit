# PandEdit TODO

### Bug Fixes
- When typing just before a function definition, the letters typed show up in the information
- After scrolling popup up and down, items are not in the same place
	- This has been worked around by setting the max number of popup lines to 7, not 8

- Using keyboard shortcut for command while reading path/name puts command name in
- Don't allow empty buffer name

### Features
- std::string_view
- UTF-8 characters

- Ask for confirmation before creating file
- Tell user when key combination is undefined
- Right-align suggestion information

- Project file
	- Can contain frame layout information
	- Specify include directories
		- Should find functions in all files
	- Can specify a compile command
