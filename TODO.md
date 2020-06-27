# PandEdit TODO

### Bug Fixes
- When typing just before a function definition, the letters typed show up in the information
- After scrolling popup up and down, items are not in the same place
	- This has been worked around by setting the max number of popup lines to 7, not 8
- Scrolling before 0 in popup causes a weird animation

### Features
- std::string_view
- UTF-8 characters

- Right-align suggestion information
- Smooth scrolling for the popup window

- Project file
	- Can contain frame layout information
	- Specify include directories
		- Should find functions in all files
	- Can specify a compile command
