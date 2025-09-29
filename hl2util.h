Constant MAX_HEALTH = 100;
Constant MAX_HEV_CHARGE = 100;

Global talks = 0;
! health ranges from 0 to MAX_HEALTH, if it reaches 0, the player dies. In certain locations, the player
! automatically heals (depending on the location), and in others, they can heal by using a medkit.
! For simplicity, the HEV suit does not have a battery level, and the player can always use the flashlight,
! and armor is not implemented.
Global health = MAX_HEALTH;
global hev_charge = MAX_HEV_CHARGE;
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
	life [;
		Ask:
			<<Talk self>>;
	],
	has animate;

Class Screen
	with watch_text 0,
	has static;

Class Breencast
	class Screen,
	with description "A large screen displaying the former administrator of Black Mesa, Dr. Breen. You can watch or listen to it.";

Class CivilProtectionUnit
	with name "civil" "protection" "officer" "cp" "cop",
	attack [ weapon;
		switch(weapon) {
		1: ! stun baton
			print (The)self;
			if (location hasnt volatile) {
				! hit the player but don't do damage
				print " hits you with the stun baton";
			} else {
				if (random(4) == 1) {
					! miss
					print " swings the stun baton at you, but misses.";
				} else {
					! hit
					print " hits you with the stun baton.";
					TakeDamage(5, -1);
				}
			}
			break;
		}
	],
	has animate male;

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

Class LocationDoor
	with description "This is a plain looking door.",
	after [;
		Look,Examine:
			print (The)self ," is ";
			if (self has open) {
				"open.";
			}
			if (self has ajar) {
				"ajar.";
			}
			print "closed";
			if (self has lockable && self has locked) {
				" and locked.";
			}
			".";
		Open,Close:
			give self ~ajar;
	],
	has door openable static;

! ----------------------------------------------------------------------------
! Helper subroutines
! ----------------------------------------------------------------------------

[ Pause text dummy;
	if (text == 0) {
		text = "Press a key to continue";
	}
	style roman;
	print "[", (string)text, "]";
	@read_char 1 dummy;
	print "^^";
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
	ExitsSub(); ! "obvious exits are..."
];

[ DrawStatusLine width score_pos health_pos moves_pos;
	if (location == nothing || location == thedark) return;
	StatusLineHeight(gg_statuswin_size);
	MoveCursor(1,1);

	width = ScreenWidth();

	if (width > 66) {
		if (wearing_hev) {
			! full size: Location Name     Health: 100  HEV: 100   Score: 999   Moves: 999
			health_pos = width - 46;
		} else {
			! full size: Location Name     Health: 100  Score: 999   Moves: 999
			health_pos = width - 36;
		}
	} else if (width > 40) {
		if (wearing_hev) {
			! smaller: Location Name  +:100  HEV:100  999/999
			health_pos = width - 30;
		} else {
			! smaller: Location Name  +:100  999/999
			health_pos = width - 20;
		}
	} else {
		! smallest: +:100  HEV:100  999/999
		! smallest: +:100  999/999
		health_pos = 2;
	}

	if (width < 53) {
		moves_pos = width - 7;
	} else {
		score_pos = width - 23;
		moves_pos = width - 11;
	}

	spaces width;

	if (width > 40) {
		MoveCursor(1, 2);
		if (location == thedark) {
			print (name) location;
		} else {
			FindVisibilityLevels();
			if (visibility_ceiling == location)
				print (name) location;
			else
				print (The) visibility_ceiling;
		}
	}

	MoveCursor(1, health_pos);
	if (width > 66) {
		if (wearing_hev) {
			print "Health: ", health, "  HEV: ", hev_charge;
		} else {
			print "Health: ", health;
		}
	} else {
		if (wearing_hev) {
			print "+:", health, "  HEV:", hev_charge;
		} else {
			print "+:", health;
		}
	}

	if (width > 66) {
		MoveCursor(1, score_pos);
		print (string) SCORE__TX, sline1;

		MoveCursor(1, moves_pos);
		print (string) MOVES__TX, sline2;
	} else {
		MoveCursor(1, moves_pos);
		print sline1, "/", sline2;
	}

	.DSLContinue;
	MainWindow();
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
		which compiles to Z-machine format. The source code is available at https://github.com/Eggbertx/HalfLife2-ZMachine.^^
		NOTE: This is a fan work, and is not affiliated with or endorsed by Valve Corporation or
		its affiliates in any way. Half-Life 2 and all related characters, names,
		locations, and other elements are the property of Valve Corporation. This demake is
		a non-commercial project made for educational and entertainment purtime_s_poss only.";
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
		-1: print "You probably should have avoided that civil protection unit"; ! killed by test CP, removed
		-2: print "You probably should have listened to Barney and left when you had the chance."; ! killed by Combine soldiers in the interrogation room
	}
	if (deadflag < 0 && deadflag == -2) {
		deadflag = -deadflag;
	} else if (deadflag < 0) {
		deadflag = 1;
	}
];

[ TakeDamage dmg flag_if_dead;
	health = health - dmg;
	if (health <= 0) {
		deadflag = flag_if_dead;
		rtrue; ! signal that the player has died
	} else if (health < 3 && wearing_hev) {
		print "Your HEV suit beeps urgently. ~Warning, vital signs critical.~";
	}
];

! ----------------------------------------------------------------------------
! Attributes
! ----------------------------------------------------------------------------

Attribute volatile; ! for locations where looking/examining is not a free action
Attribute healing; ! for locations where the player automatically heals
Attribute enclosed; ! for locations where obvious exits should not be printed
Attribute targeting_player; ! for NPCs and objects that are focused on or attacking the player
Attribute ajar;

! ----------------------------------------------------------------------------
! Verb subroutines
! ----------------------------------------------------------------------------

[ TalkSub;
	if (noun ofclass LimitedTalker) {
		if (noun.talk_speech ~= 0) {
			PrintOrRun(noun, talk_speech);
			noun.talk_speech = 0;
			talks++;
			noun.talked = true;
		} else if (noun.talk2_speech ~= 0) {
			PrintOrRun(noun, talk2_speech);
			noun.talk2_speech = 0;
		} else if (noun.talk3_speech ~= 0) {
			PrintOrRun(noun, talk3_speech);
			noun.talk3_speech = 0;
		} else if (noun.default_response ~= 0) {
			PrintOrRun(noun, default_response);
		} else {
			Pronoun(noun, "doesn't", "don't");
			print "have much ";
			if (noun.talked) print "else ";
			"to say.";
		}
		if (talks >= 2 && location == PointInsertionTrainCar) {
			give location ~enclosed;
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
	}
	"You get no response.";
];

[ WatchScreen;
	if (noun.watch_text == 0) {
		"The display appears to be nonfunctional.";
	}
	PrintOrRun(noun, watch_text);
	rtrue;
];

[ WatchSub;
	if (noun ofclass Screen) {
		WatchScreen();
		rtrue;
	}
	"It's rude to stare.";
];

[ ListenSub;
	if (noun ofclass Screen) {
		WatchScreen();
		rtrue;
	}
	"You hear nothing unexpected.";
];

[ CheckHealthSub;
	print "Health: ", health, " out of ", MAX_HEALTH, "^";
	if (wearing_hev)
		print "You are wearing the HEV suit.^";
];

[ NonsenseSub;
	print "Nothing happens. What do you think this is, an Infocom game?^";
	score--;
];

[ ActivateSub;
	"Nothing happens.";
];

[ LookDirectionSub dir_obj;
	if(ADirection(noun) == false) {
		<<Examine noun>>;
	}
	print "Looking ";
	switch(noun) {
		n_obj: dir_obj = location.n_to;
		s_obj: dir_obj = location.s_to;
		w_obj: dir_obj = location.w_to;
		e_obj: dir_obj = location.e_to;
		nw_obj: dir_obj = location.nw_to;
		ne_obj: dir_obj = location.ne_to;
		sw_obj: dir_obj = location.sw_to;
		se_obj: dir_obj = location.se_to;
		u_obj: dir_obj = location.u_to;
		d_obj: dir_obj = location.d_to;
		in_obj: dir_obj = location.in_to;
		out_obj: dir_obj = location.out_to;
	}
	if (noun == d_obj) {
		print "down";
	} else {
		print (name)noun;
	}
	print ", you see ";
	if (dir_obj ofclass Routine) {
		dir_obj = dir_obj();
	}
	if (dir_obj == 0) {
		"nothing unexpected.";
	}
	if (parent(dir_obj) == location) {
		print_ret (a)dir_obj, ".";
	}
	print_ret (the)dir_obj, ".";
];

! ----------------------------------------------------------------------------
! Grammar
! ----------------------------------------------------------------------------

Include "Grammar";

Verb "activate" "use"
	* noun -> Activate;

Verb "talk" "to"
	* "to" noun -> Talk
	* noun      -> Talk;

! used for screens, instead of behaving the same as examine
Extend "watch" replace
	* noun -> Watch
	* "the" noun -> Watch;

Extend "listen" replace
	* -> Listen
	* noun -> Listen
	* "to" noun -> Listen;

! copying and extending this from the standard library version to add look <noun> for brevity
Extend "look" replace
	*                                           -> Look
	* noun=ADirection                           -> LookDirection
	* noun                                      -> Examine
	* "at" noun                                 -> LookDirection
	* "inside"/"in"/"into"/"through"/"on" noun  -> Search
	* "under" noun                              -> LookUnder
	* "up" topic "in" noun                      -> Consult
	* "to" noun=ADirection                      -> LookDirection;

Verb "stack"
	* multiexcept "on"/"onto" noun -> PutOn;

Verb "help" "h//"
	* -> Help;

Verb "status" "health"
	* -> CheckHealth;

Verb "xyzzy" "plugh" "plover"
	* -> Nonsense;