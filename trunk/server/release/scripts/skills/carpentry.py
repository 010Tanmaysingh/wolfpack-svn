from wolfpack import console, properties
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu
from wolfpack.utilities import hex2dec, tobackpack
import random
from skills.blacksmithing import METALS

#
# Check if the character is using the right tool
#
def checktool(char, item, wearout = 0):
  if not item:
    return 0

  # Has to be in our posession
  if item.getoutmostchar() != char:
    char.socket.clilocmessage(500364)
    return 0

  # We do not allow "invulnerable" tools.
  if not item.hastag('remaining_uses'):
    char.socket.clilocmessage(1044038)
    item.delete()
    return 0

  if wearout:
    uses = int(item.gettag('remaining_uses'))
    if uses <= 1:
      char.socket.clilocmessage(1044038)
      item.delete()
      return 0
    else:
      item.settag('remaining_uses', uses - 1)

  return 1

#
# Bring up the carpentry menu
#
def onUse(char, item):
  if not checktool(char, item):
    return 1

  menu = findmenu('CARPENTRY')
  if menu:
    menu.send(char, [item.serial])
  return 1

#
# Carp an item.
# Used for scales + ingots
#
class CarpItemAction(CraftItemAction):
  def __init__(self, parent, title, itemid, definition):
    CraftItemAction.__init__(self, parent, title, itemid, definition)
    self.markable = 0 # Most carpentry items are not markable

  #
  # Check if we did an exceptional job.
  #
  def getexceptionalchance(self, player, arguments):
    # Only works if this item requires blacksmithing
    if not self.skills.has_key(CARPENTRY):
      return 0

    minskill = self.skills[CARPENTRY][0]
    maxskill = self.skills[CARPENTRY][1]

    chance = ( player.skill[CARPENTRY] - minskill ) / 10

    # chance = 0 - 100
    if chance > 100:
      chance = 100
    elif chance < 0:
      chance = chance * -1

    # chance range 0.00 - 1.00
    chance = chance * .01
    return chance

  #
  # Apply resname and color to the item.
  #
  def applyproperties(self, player, arguments, item, exceptional):
    # See if special ingots were used in the creation of
    # this item. All items crafted by carpenterss gain the
    # color!

    if self.submaterial1 > 0:
      material = self.parent.getsubmaterial1used(player, arguments)
      material = self.parent.submaterials1[material]
      item.decay = 1


    # Reduce the uses remain count
    checktool(player, wolfpack.finditem(arguments[0]), 1)

  #
  # First check if we are near an anvil and forge.
  # Then play a carpentry sound.
  #
  def make(self, player, arguments):
    assert(len(arguments) > 0, 'Arguments has to contain a tool reference.')

    if not checktool(player, wolfpack.finditem(arguments[0])):
      return 0

    return CraftItemAction.make(self, player, arguments)

  #
  # Play a simple soundeffect
  #
  def playcrafteffect(self, player, arguments):
    player.soundeffect(0x2a)

#
# A blacksmith menu. The most notable difference is the
# button for selecting another ore.
#
class CarpentryMenu(MakeMenu):
  def __init__(self, id, parent, title):
    MakeMenu.__init__(self, id, parent, title)
    self.allowmark = 1
    #self.allowrepair = 1
    self.submaterials2 = METALS
    self.submaterial2missing = 1042081 # Ingots
    #self.submaterial1missing = 1041524 # Wood
    #self.submaterial2missing = 1042598 # Boards
    #self.submaterial3missing = 1042081 # Ingots
    #self.submaterial4missing = 1042081 # Cloth
    self.submaterial1noskill = 500586
    self.gumptype = 0x4f6ba469 # This should be unique

#
# Load a menu with a given id and
# append it to the parents submenus.
#
def loadMenu(id, parent = None):
  definition = wolfpack.getdefinition(WPDT_MENU, id)
  if not definition:
    if parent:
      console.log(LOG_ERROR, "Unknown submenu %s in menu %s.\n" % (id, parent.id))
    else:
      console.log(LOG_ERROR, "Unknown menu: %s.\n" % id)
    return

  name = definition.getattribute('name', '')
  menu = CarpentryMenu(id, parent, name)

  # See if we have any submenus
  for i in range(0, definition.childcount):
    child = definition.getchild(i)
    # Submenu
    if child.name == 'menu':
      if not child.hasattribute('id'):
        console.log(LOG_ERROR, "Submenu with missing id attribute in menu %s.\n" % menu.id)
      else:
        loadMenu(child.getattribute('id'), menu)

    # Craft an item
    elif child.name == 'craft':
      if not child.hasattribute('definition') or not child.hasattribute('name'):
        console.log(LOG_ERROR, "Carpentry action without definition or name in menu %s.\n" % menu.id)
      else:
        itemdef = child.getattribute('definition')
        name = child.getattribute('name')
        try:
          # See if we can find an item id if it's not given
          if not child.hasattribute('itemid'):
            item = wolfpack.getdefinition(WPDT_ITEM, itemdef)
            itemid = 0
            if item:
              itemchild = item.findchild('id')
              if itemchild:
                itemid = itemchild.value
          else:
            itemid = hex2dec(child.getattribute('itemid', '0'))
          action = CarpItemAction(menu, name, int(itemid), itemdef)
        except:
          console.log(LOG_ERROR, "Carpentry action with invalid item id in menu %s.\n" % menu.id)

        # Process subitems
        for j in range(0, child.childcount):
          subchild = child.getchild(j)

          # How much of the primary resource should be consumed
          if subchild.name == 'wood':
            action.submaterial1 = hex2dec(subchild.getattribute('amount', '0'))

          elif subchild.name == 'boards':
            action.submaterial2 = hex2dec(subchild.getattribute('amount', '0'))

          # How much of the secondary resource should be consumed
          elif subchild.name == 'ingots':
            action.submaterial3 = hex2dec(subchild.getattribute('amount', '0'))

          elif subchild.name == 'cloth':
            action.submaterial4 = hex2dec(subchild.getattribute('amount', '0'))

          # Normal Material
          elif subchild.name == 'material':
            if not subchild.hasattribute('id'):
              console.log(LOG_ERROR, "Material element without id list in menu %s.\n" % menu.id)
              break
            else:
              ids = subchild.getattribute('id').split(';')
              try:
                amount = hex2dec(subchild.getattribute('amount', '1'))
              except:
                console.log(LOG_ERROR, "Material element with invalid id list in menu %s.\n" % menu.id)
                break
              action.materials.append([ids, amount])

          # Skill requirement
          elif subchild.name in skillnamesids:
            skill = skillnamesids[subchild.name]
            try:
              minimum = hex2dec(subchild.getattribute('min', '0'))
            except:
              console.log(LOG_ERROR, "%s element with invalid min value in menu %s.\n" % (subchild.name, menu.id))

            try:
              maximum = hex2dec(subchild.getattribute('max', '1200'))
            except:
              console.log(LOG_ERROR, "%s element with invalid max value in menu %s.\n" % (subchild.name, menu.id))

            action.skills[skill] = [minimum, maximum]

  # Sort the menu. This is important for the makehistory to make.
  menu.sort()

#
# Load the carpentry menu.
#

def onLoad():
  loadMenu('CARPENTRY')

