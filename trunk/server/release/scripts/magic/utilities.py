
import magic.spellbook
from wolfpack.utilities import *

TARGET_CHAR = 1
TARGET_ITEM = 2
TARGET_GROUND = 3
TARGET_IGNORE = 4

# Common Reagents
REAGENT_BLACKPEARL = 0xf7a
REAGENT_BLOODMOSS = 0xf7b
REAGENT_GARLIC = 0xf84
REAGENT_GINSENG = 0xf85
REAGENT_MANDRAKE = 0xf86
REAGENT_NIGHTSHADE = 0xf88
REAGENT_SULFURASH = 0xf8c
REAGENT_SPIDERSILK = 0xf8d

# Rarer Reagents
REAGENT_BATWING = 0xf78
REAGENT_BLACKMOOR = 0xf79
REAGENT_BLOODSPAWN = 0xf7c
REAGENT_DAEMONBLOOD = 0xf7d
REAGENT_BONE = 0xf7e
REAGENT_BRIMSTONE = 0xf7f
REAGENT_DAEMONBONE = 0xf8
REAGENT_FERTILEDIRT = 0xf81
REAGENT_DRAGONSBLOOD = 0xf82
REAGENT_EXECUTIONERSCAP = 0xf83
REAGENT_EYEOFNEWT = 0xf87
REAGENT_OBSIDIAN = 0xf89
REAGENT_PIGIRON = 0xf8a
REAGENT_PUMICE = 0xf8b
REAGENT_NOXCRYSTAL = 0xf8e
REAGENT_GRAVEDUST = 0xf8f
REAGENT_DEADWOOD = 0xf9
REAGENT_WYRMSHEART = 0xf91

# Casting modes
MODE_BOOK = 0
MODE_SCROLL = 1
MODE_WAND = 2

#
# Remove a possible timer/targetrequest
#
def fizzle(char):	
	char.dispel(char, 1, "cast_delay")

	eventlist = char.events
	if 'magic' in eventlist:
		eventlist.remove('magic')
		char.events = eventlist

	if char.socket:
		char.socket.deltag('cast_target')

	char.effect(0x3735, 1, 30)
	char.soundeffect(0x5c)

# Check whether the spellbook's of a char have that specific spell
def hasSpell(char, spell):
	book = char.itemonlayer(1)

	if magic.spellbook.hasspell(book, spell):
		return 1

	for book in char.getbackpack().content:
		if magic.spellbook.hasspell(book, spell):
			return 1
			
	return 0

#
# A general purpose function for chaning some users stats
# This is shared among: curse, bless, all single stat modifying spells
#
def statmodifier_dispel(char, args, source, dispelargs):
	stat = args[0]
	(amount1, amount2, amount3) = args[1:]

	if stat == 0 or stat == 3:
		char.strength2 -= amount1
		if char.strength - amount1 > 0:
			char.strength -= amount1
		else:
			char.strength = 1
		char.hitpoints = min(char.hitpoints, char.maxhitpoints)
	if stat == 1 or stat == 3:
		char.dexterity2 -= amount2
		if char.dexterity - amount2 > 0:
			char.dexterity -= amount2
		else:
			char.dexterity = 1
		char.stamina = min(char.stamina, char.maxstamina)
	if stat == 2 or stat == 3:
		char.intelligence2 -= amount3
		char.intelligence -= amount3
		char.mana = min(char.mana, char.maxmana)

	# If we're not dispelling it silently, update 
	# health and stats
	if not "silent" in dispelargs:
		if stat == 0 or stat == 3:
			char.updatehealth()
		char.updatestats()

	return

#
# A stat enhancing spell has expired
#
def statmodifier_expire(char, args):
	stat = args[0]
	(amount1, amount2, amount3) = args[1:]
	
	if stat == 0 or stat == 3:
		char.strength2 -= amount1
		if char.strength - amount1 > 0:
			char.strength -= amount1
		else:
			char.strength = 1
		char.hitpoints = min(char.hitpoints, char.maxhitpoints)
		char.updatehealth()
	if stat == 1 or stat == 3:
		char.dexterity2 -= amount2
		if char.dexterity - amount2 > 0:
			char.dexterity -= amount2
		else:
			char.dexterity = 1
		char.stamina = min(char.stamina, char.maxstamina)		
	if stat == 2 or stat == 3:
		char.intelligence2 -= amount3
		char.intelligence -= amount3
		char.mana = min(char.mana, char.maxmana)

	char.updatestats()
	return

#
# Apply a stat modifying spell
#
def statmodifier(char, target, stat, curse):
	duration = min(15, int(char.skill[MAGERY] * 0.12)) * 1000
	amount = min(5, 1 + int(char.skill[MAGERY] * 0.01))
	
	# Reverse if it's a curse
	if curse:
		amount *= -1
	
	# Dispel any old stat modifiers on this character
	# And readd a new one (remove the old ones silently)
	char.dispel(char, 0, "magic_statmodifier", ["silent"])

	if stat == 0 or stat == 3:
		# Adjust amount
		if target.strength + amount < 1:
			amount1 = -(target.strength - 1)
		else:
			amount1 = amount
	
		target.strength2 += amount	
		target.strength += amount
		target.hitpoints = min(target.hitpoints, target.maxhitpoints)
		target.updatehealth()		
	if stat == 1 or stat == 3:
		if target.dexterity + amount < 1:
			amount2 = -(target.dexterity - 1)
		else:
			amount2 = amount

		target.dexterity2 += amount
		target.dexterity += amount
		target.stamina = min(target.stamina, target.maxstamina)
	if stat == 2 or stat == 3:
		if target.intelligence + amount < 1:
			amount3 = -(target.intelligence - 1)
		else:
			amount3 = amount

		target.intelligence2 += amount
		target.intelligence += amount
		target.mana = min(target.mana, target.maxmana)

	target.updatestats()

	# Save the values in a tempeffect to remove them later
	target.addtimer(duration, "magic.utilities.statmodifier_expire", [stat, amount1, amount2, amount3], \
		1, 1, "magic_statmodifier", "magic.utilities.statmodifier_dispel")

#
# When a fieldeffect spell expires.
#
def field_expire(object, args):
	for serial in args:
		item = wolfpack.finditem(serial)
		if item:
			item.delete()

