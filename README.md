# phpPad
Code editor for PHP

Introduction
This code editor is specialized for projects in php. Currently the editor is in an alpha state but except from a search and replace function feature complete. The current fea-tures still need to be tested in depth before they will be expanded.
Features
-	Have several documents open in tabs
-	Code highlighting for php, css, javascript and html in the same file
-	Autocomplete for (custom) function names, variable names and brackets
-	Variables and functions from included files will be completed as well
-	File explorer to organize your files in projects
-	Restore function
-	Insert function for standard html components like tables or images
-	Reset files to its initial state
-	Restore a closed tab

Installation
To install the phpPad code editor use one of the provided installers or download the source code and compile it with Qt Creator.

First run
After you installed phpPad you can either start with creating new documents or open existing documents. It is recommended to create or open a project first. A project con-sists of a root folder on your hard drive with your files and folders inside. If you open a project or create a project the root folder and its content will be displayed on the right side. The project explorer gives you the same functionality as a basic file explorer. You can create files and folders to structure your project.
Editing php files
If you open a php file all files which are included by include or include_once will be listed on the bottom of the right side. Variable and function names from those files are added to the completer. To update that list save your file.

Known Bugs
-	Highlighting in css files does not work properly if a line is not closed by a semico-lon.
-	The AJAX button is not implemented yet.
