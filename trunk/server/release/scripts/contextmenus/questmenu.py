#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: MagnusBr                       #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################
import speech
from system.quest import *

def onContextEntry( char, target, tag  ):

	if ( tag == 1 ):
		npcquestmain(target, char)

	return 1
