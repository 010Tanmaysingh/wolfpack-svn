#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Sep, 27 '03	        #
#################################################################

from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from skills.mining import *
from wolfpack.time import *
from wolfpack.utilities import *


def onUse( char, tool ):
	#Already digging ?
	if char.socket.hastag( 'is_mining' ) and ( char.socket.gettag( 'is_mining' ) > servertime() ):
		# You are already digging.
		char.socket.clilocmessage( 503029, "", GRAY ) 
		return OK
	
	# Can't mine on horses
	if char.itemonlayer( LAYER_MOUNT ):
		# You can't mine while riding.
		char.socket.clilocmessage( 501864, "", GRAY ) 
		return OK
	
	# Who is tool owner ?
	if tool.getoutmostchar() != char:
		# You can't use that, it belongs to someone else
		char.socket.clilocmessage( 500364, "", GRAY ) 
		return OK
	
	# Is that mining tool ?
	if isminingtool( tool ):
		# Where do you wish to dig?
		char.socket.clilocmessage( 503033, "", GRAY) 
		char.socket.attachtarget( "skills.mining.response", [ tool ] )
	else:
		char.socket.clilocmessage( 500735, "", GRAY) # Don't play with things you don't know about. :)
		return OK
	
	return OK
