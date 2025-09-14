Constant MAX_HEALTH 5;

Global talks = 0;
Global train_arrived = false;
! health ranges from 0 to MAX_HEALTH, if it reaches 0, the player dies. In certain locations, the player
! automatically heals (depending on the location), and in others, they can heal by using a medkit.
! For simplicity, the HEV suit does not have a battery level, and the player can always use the flashlight,
! and armor is not implemented.
Global health = MAX_HEALTH;
Global wearing_hev = false;

! ----------------------------------------------------------------------------
! Classes
! ----------------------------------------------------------------------------

! a person (or persons) who the user can talk to up to 3 times, after that they will respond with default_response if set,
! otherwise they will say they have nothing more to say.
Class LimitedTalker
	with talk_speech 0,
	talk2_speech 0,
	talk3_speech 0,
	default_response 0,
	talked false,
	before [;
		Talk,Ask:
			if (self.talk_speech ~= 0) {
				PrintOrRun(self, talk_speech);
				self.talk_speech = 0;
				talks++;
				self.talked = true;
			} else if (self.talk2_speech ~= 0) {
				PrintOrRun(self, talk2_speech);
				self.talk2_speech = 0;
			} else if (self.talk3_speech ~= 0) {
				PrintOrRun(self, talk3_speech);
				self.talk3_speech = 0;
			} else if (self.default_response ~= 0) {
				PrintOrRun(self, default_response);
			} else {
				Pronoun(self, "doesn't", "don't");
				print "have much ";
				if (self.talked) print "else ";
				"to say.";
			}
			if (talks >= 2 && location == PointInsertionTrainCar) {
				train_arrived = true;
				! give location ~enclosed; ! allow the player to exit the train car
				remove DidntSeeYouGetOnCitizen;
				remove RelocatedCitizen;
				print "^";
				Pause("The train has arrived.");
				print "^The train comes to a halt, and the doors open. The man by the door mutters,
					~Well, end of the line~ and gets off. The man sitting down gets up and departs as well.^
					A robotic voice states ";
				style bold;
				print "~Exit the train.~^";
				style roman;
			}
			rtrue;
	],
	says_verb 0,
	has animate;


Class BreenCast
	with description "A large screen displaying the former administrator of Black Mesa, Dr. Breen. You can watch or listen to it.",
	watch_breencast 0,
	before [;
		Watch,Listen:
			if (self.watch_breencast == 0) {
				! this shouldn't normally happen, but just in case
				"The display appears to be nonfunctional.";
			}
			PrintOrRun(self, watch_breencast);
			rtrue;
	],
	has static;

Class GameLocation
	with danger_turn  -1, ! if volatile and danger_turn > -1, something bad will happen when turns >= danger_turn
	after [;
		! Look:
		! 	self.look_base();
			! object of this class should handle decrementing danger_turn if volatile

		if (self has healing && health < MAX_HEALTH) {
			health++;
		}
	],
	! look_base [;
	! 	if (self has volatile) {
	! 		"This area seems dangerous. You should move or do something quickly.";
	! 	} else if (turns > 0) {
	! 		! unlike most text adventures, looking around does not always count as a turn
	! 		! only if the location is volatile (i.e. something bad will happen if you don't move or do something)
	! 		turns--;
	! 	}
	! ],
	has light;

! ----------------------------------------------------------------------------
! Helper subroutines
! ----------------------------------------------------------------------------

[ Pause text dummy;
	style roman;
	print "[";
	if (text == 0) {
		print "Press a key to continue";
	} else {
		print (string) text;
	}
	print "]";
	@read_char 1 dummy;
	print "^^";
];

! Confirm prints the given text and waits for a Y or N response, returning true for Y and false for N.
! If the user presses any other key, it will reprompt if default_is_no is false, or return false if default_is_no is true.
[ Confirm text default_is_no yn;
	while(true) {
		yn = Pause(text);
		switch(yn) {
			89,121: ! Y or y
				rtrue;
			78,110: ! N or n
				rfalse;
			default:
				if (default_is_no)
					rfalse;
		}
	}
];

! Print a pronoun for a person, based on the object's attributes, "he" for male, "she" for female, "it" for neuter,
! and "they" if none are set. The ifgendered parameter is printed (if set) after he, she, or it, and the ifthey parameter
! is printed after they.
! Example usage: PrintPronoun(player, 'is', 'are');
! Output: He is ... or They are ...
! Note that the parameters should do not need leading or trailing spaces.
[ Pronoun person ifgendered ifthey;
	if (person has male) {
		print "He";
		if(ifgendered ~= 0)
			print " ", (string)ifgendered, " ";
	} else if (person has female) {
		print "She";
		if (ifgendered ~= 0)
			print " ", (string)ifgendered, " ";
	} else if (person has neuter) {
		print "It";
		if (ifgendered ~= 0)
			print " ", (string)ifgendered, " ";
	} else {
		print "They";
		if (ifthey ~= 0) {
			print " ", (string)ifthey, " ";
		}
	}
];

[ ExitsSub i dir dest;
	print "Obvious exits: ";
	i = 0;
	objectloop (dir ofclass CompassDirection) {
		dest = location.(dir.door_dir);  ! Look up the property (like n_to, s_to, etc.)
		if (dest) {
			if (i++ > 0) print ", ";
			print (name) dir;
		}
	}
	! if (i == 0) print "none";
	new_line;
];

[ LookRoutine;
	if (location == nothing || location == thedark || location has enclosed) return;
	! if (location has visited) print (name) location, "^";
	! else print (name) location, " (first time seen)^";
	new_line;

	if (location has volatile) {
		"This area seems dangerous. You should move or do something soon.";
	} else if (turns > 0) {
		! unlike most text adventures, looking around does not always count as a turn
		! only if the location is volatile (i.e. something bad will happen if you don't move or do something)
		turns--;
	}
	<<Exits>>; ! "obvious exits are..."
];


! ----------------------------------------------------------------------------
! Help menu and subroutines
! ----------------------------------------------------------------------------

[ HelpMenu;
	if (menu_item == 0) {
		item_width = 11;
		item_name = "Main Menu";
		return 3; ! number of items in the menu
	}
	if (menu_item == 1) {
		item_width = 6;
		item_name = "Instructions";
	}
	if (menu_item == 2) {
		item_width = 4;
		item_name = "About";
	}
	if (menu_item == 3) {
		item_width = 4;
		item_name = "Tips";
	}
];

[ HelpInfo;
	if (menu_item == 1) {
		! Instructions for playing
		print "This is a text adventure demake of Half-Life 2.^
		^As in most text adventure games, you can interact with the world around you with simple text commands, usually
		of the form <verb>, <verb> <noun>, or <verb> <descriptive noun>, with optional abbreviations. For example, to move north, you can type ~go north~,
		~move north~, ~go n~, ~north~, or ~n~. In addition, you can also move in other directions, such as the other cardinal directions, as well as
		up, down, in, out, etc, depending on the location and available exits. Some locations may have special movement commands,
		such as enter, exit, jump, etc. You can check your health with the ~health~ command.^
		You can get a list of available directions by typing ~look~. or look at a specific thing or person by typing
		~look/examine <name>~ (without quotes).^^
		You can interact with objects and people by typing commands such as ~take/get <object>~, ~use <object>~,  ~talk person~,
		~hit <person>~ etc., reaping the benefits of clever thinking, and the consequences of rash actions.^^
		Note: Unlike most text adventure games, looking at the location or an object will not always be treated as a move/turn
		representing an in-universe action causing time to pass. However, some cases require the player to move or act
		with haste, such as escaping from enemies. In these cases, you will be informed that time is passing, and the
		location will have the volatile attribute set. There will also be environmental cues that if you do not do something,
		bad things may happen.^^";
	} else if (menu_item == 2) {
		! About this demake
		print "This is a text adventure demake of Half-Life 2. It was made using the Inform 6 programming language,
		which compiles to Z-machine format. The source code is available at https://github.com/Eggbertx/HalfLife2-ZMachine.";
	} else if (menu_item == 3) {
		! Tips and tricks
		print "Some tips and tricks for playing this game:^
		Save often, especially before risky actions. Unlike Half Life 2, Z-Machines do not automatically
		save your progress. You can do this by typing the ~save~ command. If you die, you can load your
		saved games by typing the ~restore~ command.^^
		If you get stuck and don't know what to do, try looking around or examining objects in the environment.
		I did my best to give plenty of textual and verbal cues as to what you can do in a given location, though text
		adventures are inherently more limited than 3D games. and require some imagination and intuition.^^
		Some characters may have more to say if you talk to them multiple times.^^
		Use the ~help~ command to access this menu again. This will never count as a action, even in
		volatile locations.^^
		Remember that some actions may have consequences, as in the original game, so think before you act. If
		a civil protection officer is nearby, trying to interact with them beyond looking may incur their wrath.^";
	}
];

[ HelpSub;
	DoMenu("There is information provided on the following:^
		^    Instructions for playing
		^    About this demake
		^    Tips and tricks^", HelpMenu, HelpInfo, 0, true);
	turns--; ! prevent the turn counter from incrementing
];

[ AfterLife;
	switch (deadflag) {
		1: "You probably should have avoided that civil protection unit";
		default: "You died.";
	}
];

[ TakeDamage dmg flag_if_dead;
	health = health - dmg;
	if (health <= 0) {
		deadflag = flag_if_dead;
		rtrue; ! signal that the player has died
	} else if (health < 3 && wearing_hev) {
		"Your HEV suit beeps urgently. ~Warning, vital signs critical.~";
	}
];

! ----------------------------------------------------------------------------
! Attributes
! ----------------------------------------------------------------------------

Attribute volatile; ! for locations where looking/examining is not a free action
Attribute healing; ! for locations where the player automatically heals
Attribute enclosed; ! for locations where obvious exits should not be printed

! ----------------------------------------------------------------------------
! Verb subroutines
! ----------------------------------------------------------------------------

[ TalkSub o;
	if (o == 0) {
		"You get no response.";
	}
	if (o ofclass LimitedTalker) {
		! handled in the before property of LimitedTalker
		rfalse;
	}
	"It doesn't have much to say.";
];

[ WatchSub;
];

[ CheckHealthSub;
	print "Health: ", health, " out of ", MAX_HEALTH, "^";
	if (wearing_hev)
		print "You are wearing the HEV suit.^";
];

! ----------------------------------------------------------------------------
! Grammar
! ----------------------------------------------------------------------------

Include "Grammar";
! Include "./ExpertGrammar";


Verb 'talk' 't//'
	* 'to' noun -> Talk
	* noun      -> Talk;

! used for breencasts, instead of behaving the same as examine
Extend 'watch' replace
	* noun -> Watch;

! copying and extending this from the standard library version to add look <noun> for brevity
Extend 'look' replace
	*                                           -> Look
	* noun                                      -> Examine
	* 'at' noun                                 -> Examine
	* 'inside'/'in'/'into'/'through'/'on' noun  -> Search
	* 'under' noun                              -> LookUnder
	* 'up' topic 'in' noun                      -> Consult
	* noun=ADirection                           -> Examine
	* 'to' noun=ADirection                      -> Examine;

Verb 'help' 'h//'
	* -> Help;

Verb 'status' 'health'
	* -> CheckHealth;
