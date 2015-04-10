# Truss Scheduler

Just putting it here so I have all my projects in one place.
This reads a very limited subset of MiTek's TRE files and creates diagrams
  on a virtual truck bed which can then be placed on the calendar.
It also supports a SQLite database to store and retrieve old jobs.

Requires wxWindows and SQLite to build, and you'll have to update the makefiles.
The coding style is bad, I know - it was one of my first major C++ projects.

![Main window](/screenshots/mainscreen.png)

![Truck bed view](/screenshots/bedview.png)

![Database editor](/screenshots/database.png)

[Example printout](/screenshots/printedschedule.pdf)
