# PandEdit TODO

### Bug Fixes
- When typing just before a function definition, the letters typed show up in the information
- After scrolling popup up and down, items are not in the same place
	- This has been worked around by setting the max number of popup lines to 7, not 8

### Features
- std::string_view
- UTF-8 characters

- Mark a buffer as read-only
- Update compile buffer as compilation occurs
- Add visual indicator (asterisk) if a buffer has been modified
	- Option to revert buffer back to its state on disk

- Project file
	- Can contain frame layout information
	- Specify include directories
		- Should find functions in all files
	- Can specify a compile command
