omega implementor's notes.

Version 0.80

This version is a bugfixed version of 0.75. Hopefully not many bugs
still remain - I tried to be fairly thorough.

Notes on Game Structure:

I'm now keeping track of the last edit in date.h -- if you hack
omega, you might conceivably want to do the same. The #definition
of LAST_OMEGA_EDIT_DATE is printed by the 'V' command....

NB.  Since Laurence isn't working on omega any more, the following
wishlists are unlikely to be fulfilled,.. oh well.  I'm leaving them
in so that enthusiastic future generations can be inspired and perhaps
complete some of the ideas.

Right now the structure of guilds, dungeons, quests, etc. is not
really the way I want it. The player winds up doing basically all the
same things each game, no matter which guild he may be in, because
there is not enough detail to the world of omega. It is a LOT of
painstaking work to sort out threads of narrative structure (such as
for example the beginning attempts I've made with the LawBringer and
the way he interacts with the Circle of Sorcerors and the Order of
Paladins).

Theoretically, one need not do any questing at all to become a Total
Winner (ie, become an adept); this is also not desirable.

There are any number of ways to mechanically "crack" the game, i.e.,
to do something to take advantage of omega for the player's benefit in
some deviant way. At least you can't drop a 32 bit number of negative
gold pieces like in hack.... Anyhow, making sure these loopholes are
filled is a low priority except when they are really blatant; I'd much
rather enhance game play for people who don't cheat....

At the moment I have several goals for the next major version of
omega (1.00):

	* Enhanced game structure -- more dungeons, more detail
	within guilds, pristhoods, etc. that will make the game
	very different for different characters.

	* More player interaction. I still don't intend to have
	omega be an online multiplayer game, but it would be nice
	to have more effect on the game from the actions of 
	previous characters.

	* X and/or NeWS and/or Sunview II front-end(s). It would be nice
	to have a separate inventory window, be able to click destinations,
	have monster and terrain fonts, etc. If I get really
	overexcited, how about a perspective view of the dungeon....

As always, I welcome suggestions. It is very easy to add more monsters,
magic items, magic spells, etc....

Notes on the Code:

As far as I can tell, all this should work on any Unix* system, but
I've never tried it on anything but a Sun, not having enough disk
space to my name anywhere else to try.... It uses curses for I/O, and
tries to use as few OS-dependent functions as it can elsewhere. It
should be pretty easy to port to any Unix* variant that has curses.
omega has now been implemented not only on Suns, Vaxen, and Pyramids,
but even on machines such as Amdahls running unix over the normal
operating system. The current version has #ifdef's for MSDOS,
courtesy nathan@brokaw.lcs.mit.edu. Unfortunately, this makes
the code harder to read, but such is life.

Identifier Convention:
UPPERCASE CONSTANTS are all #defined in defs.h
Capitalized Globals are all defined in omega.c and externally declared in
    glob.h
other identifiers are lowercase

I have attempted to keep extremely system-dependent stuff segregated
from the rest of the code.

For example, all screen manipulation functions are in scr.c, for
those of you who want to port omega to something that doesn't have
curses. (Well, there's a little in file.c)

Almost all file manipulation is done in file.c, and I have attempted
to keep this simple and consistent.

A few other system calls are in omega.c and util.c, and that's about it.

Unfortunately, commenting is pretty minimal, mostly stupid things like:

  /* this routine shows a monster */
  void showmonster(m)

...and abstruse reminders to myself. As I clean the code up for the
final release version, this will improve. I usually try to avoid
cleverness in my c code, preferring two assignments and an if
statement to a single complex line with three nested conditional
expressions. Still, there are some places where the code is pretty
opaque... sorry about that. Also, I will probably switch over
to ANSI style c pretty soon.

Each module has a short description of its contents at the top, and
mostly functions tend to stay where it is reasonable to find them.
There are a few grab-bag modules, though, such as oaux1, oaux2 and
oaux3 (utility functions for ocom and other modules), oetc (total
grab-bag), and outil (mostly more general utility functions used by
everything).  To save on module size, I have broken a couple of
modules (such as ocom and osite) into pieces. I never got around to
stating precisely which functions that used to be in ocom are now in
ocom1, ocom2, or ocom3, but grep and ^s always work.... Anyhow,
one of these days I will clean up. Really.

oextern has all the function prototype declarations.

defs.h has all the CPP stuff, structure definitions, and typedefs.

glob.h has all the external global variable declarations, and
also includes the preceding two header files.

Those of you wishing to port omega to machines without virtual memory
will probably want to keep only one dungeon level in memory at a time;
you ought to be able to use the save_level and restore_level routines
in lev.c to get that working. A lot of memory (primary and
secondary), is currently required to play a full game; you could
probably run for a while with a good deal less, though. However,
you'll need a good deal of disk space if you don't have a lot of
memory; it's just not practicable to play with under several meg of
combined memory and disk space. Actually, I should probably make
up standard #define to save all levels but the current one to disk
as an option (rendering the moria-like lossage of previously entered
dungeons nugatory), but see the previous note about laziness....

KNOWN BUGS AND MISFEATURES: 

	* It is occasionally possible to destroy a game on save/restore;
	you save, and on restore you get a core dump signal. Sometimes
	when you save again it goes away; sometimes it doesn't.

	* Sometimes there seems to be an (unintentional) party room
	effect in some rooms in dungeon levels, ie, a bunch of monsters
	are having a party. I am not certain why this happens, but
	I guess it's just another one of the risks the adventurer takes...

	* The high score file occasionally gets destroyed - I presume this
	is a simultanious update problem.  I've put in file locking, and
	I hope this will rectify the problem... time will tell.

	* I've had reports of the clock in the upper right-hand side of
	the screen vanishing, and not reappearing even if the game is
	saved and then restored.  I've been unable to duplicate this,
	and so can't really address the problem, although I've restructured
	the display routines to make it cleaner - that may have some
	positive effect...

	* As far as I know (ha ha), there aren't any core dump bugs
	in actual play at the moment.

UNKNOWN BUGS:

There are an awful lot of features and the number of game states you
can get into is very large. In addition, many of the more
"interesting" bugs will probably only occur after long periods of play
since the "higher level" a feature, effect, or state is, the less
it's probably been tested. While I have not encountered any
core-dump bugs recently, there undoubtably are some, not to mention
lesser problems such as hacks to get high scores mechanically,
misfeatures, poor game interface occasions, etc.

I am always anxious to hear about any problems you have had with
omega, be they problems with compilation, gameplay, or whatever.
Suggestions for fixes are always welcome, as are wish-lists for
additional features. The most useful things are new monsters, new
items, and new magical effects, as these can be added without
disrupting existing features.

I hope you enjoy the game.

-Laurence,

			 Laurence R. Brothers
		      brothers@paul.rutgers.edu
            {anywhere}!rutgers!paul.rutgers.edu!brothers
		       "One life -- one arrow."

	(see the comment in COMPILE.ALL re: questions about omega)

* Ha, I bet you were expecting some different message. Well, this is not
a commercial document or a public net announcement, so I don't see
why I have to credit Bell Labs or UCB. So there.
