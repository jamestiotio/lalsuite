/*
 * Copyright (C) 2005 Reinhard Prix
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with with program; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 */

/**
 * \author Reinhard Prix
 * \date 2005
 * \file 
 * \ingroup SFTfileIO
 * \brief Utility functions for handling of SFTtype and SFTVector's.
 *
 * $Id$
 *
 */

/*---------- INCLUDES ----------*/
#include <lal/AVFactories.h>
#include <lal/SeqFactories.h>


#include "SFTutils.h"

NRCSID( SFTUTILSC, "$Id$" );

/*---------- DEFINES ----------*/
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define TRUE (1==1)
#define FALSE (1==0)

/*----- SWITCHES -----*/

/*---------- internal types ----------*/

/*---------- empty initializers ---------- */
LALStatus empty_status;

/*---------- Global variables ----------*/

/*---------- internal prototypes ----------*/
static int create_nautilus_site ( LALDetector *Detector );

/*==================== FUNCTION DEFINITIONS ====================*/

/** Create one SFT-struct. Allows for numBins == 0.
 */
void
LALCreateSFTtype (LALStatus *status, 
		  SFTtype **output, 	/**< [out] allocated SFT-struct */
		  UINT4 numBins)	/**< number of frequency-bins */
{
  SFTtype *sft = NULL;

  INITSTATUS( status, "LALCreateSFTtype", SFTUTILSC);
  ATTATCHSTATUSPTR( status );

  ASSERT (output != NULL, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);
  ASSERT (*output == NULL, status, SFTUTILS_ENONULL,  SFTUTILS_MSGENONULL);

  sft = LALCalloc (1, sizeof(*sft) );
  if (sft == NULL) {
    ABORT (status, SFTUTILS_EMEM, SFTUTILS_MSGEMEM);
  }

  if ( numBins )
    {
      LALCCreateVector (status->statusPtr, &(sft->data), numBins);
      BEGINFAIL (status) { 
	LALFree (sft);
      } ENDFAIL (status);
    }
  else
    sft->data = NULL;	/* no data, just header */

  *output = sft;

  DETATCHSTATUSPTR( status );
  RETURN (status);

} /* LALCreateSFTtype() */


/** Create a whole vector of \c numSFT SFTs with \c SFTlen frequency-bins 
 */
void
LALCreateSFTVector (LALStatus *status, 
		    SFTVector **output, /**< [out] allocated SFT-vector */
		    UINT4 numSFTs, 	/**< number of SFTs */
		    UINT4 numBins)	/**< number of frequency-bins per SFT */
{
  UINT4 iSFT, j;
  SFTVector *vect;	/* vector to be returned */

  INITSTATUS( status, "LALCreateSFTVector", SFTUTILSC);
  ATTATCHSTATUSPTR( status );

  ASSERT (output != NULL, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);
  ASSERT (*output == NULL, status, SFTUTILS_ENONULL,  SFTUTILS_MSGENONULL);

  vect = LALCalloc (1, sizeof(*vect) );
  if (vect == NULL) {
    ABORT (status, SFTUTILS_EMEM, SFTUTILS_MSGEMEM);
  }

  vect->length = numSFTs;

  vect->data = LALCalloc (1, numSFTs * sizeof ( *vect->data ) );
  if (vect->data == NULL) {
    LALFree (vect);
    ABORT (status, SFTUTILS_EMEM, SFTUTILS_MSGEMEM);
  }

  for (iSFT=0; iSFT < numSFTs; iSFT ++)
    {
      COMPLEX8Vector *data = NULL;

      /* allow SFTs with 0 bins: only header */
      if ( numBins )
	{
	  LALCCreateVector (status->statusPtr, &data , numBins);
	  BEGINFAIL (status)  /* crap, we have to de-allocate as far as we got so far... */
	    {
	      for (j=0; j<iSFT; j++)
		LALCDestroyVector (status->statusPtr, (COMPLEX8Vector**)&(vect->data[j].data) );
	      LALFree (vect->data);
	      LALFree (vect);
	    } ENDFAIL (status);
	}
      
      vect->data[iSFT].data = data;

    } /* for iSFT < numSFTs */

  *output = vect;

  DETATCHSTATUSPTR( status );
  RETURN (status);

} /* LALCreateSFTVector() */

/** Destroy an SFT-struct.
 */
void
LALDestroySFTtype (LALStatus *status, 
		   SFTtype **sft)	/**< SFT-struct to free */
{

  INITSTATUS( status, "LALDestroySFTtype", SFTUTILSC);
  ATTATCHSTATUSPTR (status);

  ASSERT (sft != NULL, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);

  /* be flexible: if points to null, nothing to do here.. (like 'free()') */
  if (*sft == NULL) {
    DETATCHSTATUSPTR( status );
    RETURN (status);
  }

  if ( (*sft)->data )
    {
      if ( (*sft)->data->data )
	LALFree ( (*sft)->data->data );
      LALFree ( (*sft)->data );
    }
  
  LALFree ( (*sft) );

  *sft = NULL;

  DETATCHSTATUSPTR( status );
  RETURN (status);

} /* LALDestroySFTtype() */


/** Destroy an SFT-vector
 */
void
LALDestroySFTVector (LALStatus *status, 
		     SFTVector **vect)	/**< the SFT-vector to free */
{
  UINT4 i;
  SFTtype *sft;

  INITSTATUS( status, "LALDestroySFTVector", SFTUTILSC);
  ATTATCHSTATUSPTR( status );

  ASSERT (vect != NULL, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);
  ASSERT (*vect != NULL, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);

  
  for (i=0; i < (*vect)->length; i++)
    {
      sft = &( (*vect)->data[i] );
      if ( sft->data )
	{
	  if ( sft->data->data )
	    LALFree ( sft->data->data );
	  LALFree ( sft->data );
	}
    }

  LALFree ( (*vect)->data );
  LALFree ( *vect );

  *vect = NULL;

  DETATCHSTATUSPTR( status );
  RETURN (status);

} /* LALDestroySFTVector() */


/** Copy an entire SFT-type into another. 
 * We require the destination-SFT to have a NULL data-entry, as the  
 * corresponding data-vector will be allocated here and copied into
 *
 * Note: the source-SFT is allowed to have a NULL data-entry,
 * in which case only the header is copied.
 */
void
LALCopySFT (LALStatus *status, 
	    SFTtype *dest, 	/**< [out] copied SFT (needs to be allocated already) */
	    const SFTtype *src)	/**< input-SFT to be copied */
{


  INITSTATUS( status, "LALCopySFT", SFTUTILSC);
  ATTATCHSTATUSPTR ( status );

  ASSERT (dest,  status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);
  ASSERT (dest->data == NULL, status, SFTUTILS_ENONULL, SFTUTILS_MSGENONULL );
  ASSERT (src, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);

  /* copy complete head (including data-pointer, but this will be separately alloc'ed and copied in the next step) */
  memcpy ( dest, src, sizeof(*dest) );

  /* copy data (if there's any )*/
  if ( src->data )
    {
      UINT4 numBins = src->data->length;      
      if ( (dest->data = XLALCreateCOMPLEX8Vector ( numBins )) == NULL ) {
	ABORT ( status, SFTUTILS_EMEM, SFTUTILS_MSGEMEM ); 
      }
      memcpy (dest->data->data, src->data->data, numBins * sizeof (src->data->data[0]));
    }

  DETATCHSTATUSPTR (status);
  RETURN (status);

} /* LALCopySFT() */


/** Concatenate two SFT-vectors to a new one.
 *
 * Note: the input-vectors are *copied* completely, so they can safely
 * be free'ed after concatenation. 
 */
void
LALConcatSFTVectors (LALStatus *status,
		     SFTVector **outVect,	/**< [out] concatenated SFT-vector */
		     const SFTVector *inVect1,	/**< input-vector 1 */
		     const SFTVector *inVect2 ) /**< input-vector 2 */
{
  UINT4 numBins1, numBins2;
  UINT4 numSFTs1, numSFTs2;
  UINT4 i;
  SFTVector *ret = NULL;

  INITSTATUS( status, "LALDestroySFTVector", SFTUTILSC);
  ATTATCHSTATUSPTR (status); 

  ASSERT (outVect,  status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);
  ASSERT ( *outVect == NULL,  status, SFTUTILS_ENONULL,  SFTUTILS_MSGENONULL);
  ASSERT (inVect1 && inVect1->data, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);
  ASSERT (inVect2 && inVect2->data, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);


  /* NOTE: we allow SFT-entries with NULL data-entries, i.e. carrying only the headers */
  if ( inVect1->data[0].data )
    numBins1 = inVect1->data[0].data->length;
  else
    numBins1 = 0;
  if ( inVect2->data[0].data )
    numBins2 = inVect2->data[0].data->length;
  else
    numBins2 = 0;

  if ( numBins1 != numBins2 )
    {
      LALPrintError ("\nERROR: the SFT-vectors must have the same number of frequency-bins!\n\n");
      ABORT ( status, SFTUTILS_EINPUT,  SFTUTILS_MSGEINPUT);
    }
  
  numSFTs1 = inVect1 -> length;
  numSFTs2 = inVect2 -> length;

  TRY ( LALCreateSFTVector ( status->statusPtr, &ret, numSFTs1 + numSFTs2, numBins1 ), status );
  
  /* copy the *complete* SFTs (header+ data !) one-by-one */
  for (i=0; i < numSFTs1; i ++)
    {
      LALCopySFT ( status->statusPtr, &(ret->data[i]), &(inVect1->data[i]) );
      BEGINFAIL ( status ) {
	LALDestroySFTVector ( status->statusPtr, &ret );
      } ENDFAIL(status);
    } /* for i < numSFTs1 */

  for (i=0; i < numSFTs2; i ++)
    {
      LALCopySFT ( status->statusPtr, &(ret->data[numSFTs1 + i]), &(inVect2->data[i]) );
      BEGINFAIL ( status ) {
	LALDestroySFTVector ( status->statusPtr, &ret );
      } ENDFAIL(status);
    } /* for i < numSFTs1 */

  
  DETATCHSTATUSPTR (status); 
  RETURN (status);

} /* LALConcatSFTVectors() */



/** Append the given SFTtype to the SFT-vector (no SFT-specific checks are done!) */
void
LALAppendSFT2Vector (LALStatus *status,
		     SFTVector *vect,		/**< destinatino SFTVector to append to */
		     const SFTtype *sft)	/**< the SFT to append */
{
  UINT4 oldlen;
  INITSTATUS( status, "LALAppendSFT2Vector", SFTUTILSC);
  ATTATCHSTATUSPTR (status); 

  ASSERT ( sft, status, SFTUTILS_ENULL, SFTUTILS_MSGENULL );
  ASSERT ( vect, status, SFTUTILS_ENULL, SFTUTILS_MSGENULL );

  oldlen = vect->length;
  
  if ( (vect->data = LALRealloc ( vect->data, (oldlen + 1)*sizeof( *vect->data ) )) == NULL ) {
    ABORT ( status, SFTUTILS_EMEM, SFTUTILS_MSGEMEM ); 
  }
  memset ( &(vect->data[oldlen]), 0, sizeof( vect->data[0] ) );
  vect->length ++;

  TRY ( LALCopySFT( status->statusPtr, &vect->data[oldlen], sft ), status);

  DETATCHSTATUSPTR(status);
  RETURN(status);

} /* LALAppendSFT2Vector() */



/** Append the given string to the string-vector */
void
LALAppendString2Vector (LALStatus *status,
			LALStringVector *vect,	/**< destination vector to append to */
			const CHAR *string)	/**< string to append */
{
  UINT4 oldlen;
  INITSTATUS( status, "LALAppendString2Vector", SFTUTILSC);

  ASSERT ( string, status, SFTUTILS_ENULL, SFTUTILS_MSGENULL );
  ASSERT ( vect, status, SFTUTILS_ENULL, SFTUTILS_MSGENULL );

  oldlen = vect->length;
  
  if ( (vect->data = LALRealloc ( vect->data, (oldlen + 1)*sizeof( *vect->data ) )) == NULL ) {
    ABORT ( status, SFTUTILS_EMEM, SFTUTILS_MSGEMEM ); 
  }
  vect->length ++;

  if ( (vect->data[oldlen] = LALCalloc(1, strlen(string) + 1 )) == NULL ) {
    ABORT ( status, SFTUTILS_EMEM, SFTUTILS_MSGEMEM ); 
  }

  strcpy ( vect->data[oldlen], string );
  
  RETURN(status);

} /* LALAppendString2Vector() */



/** Free a string-vector ;) */
void
LALDestroyStringVector ( LALStatus *status,
			 LALStringVector **vect )
{
  UINT4 i;
  INITSTATUS( status, "LALDestroyStringVector", SFTUTILSC);

  ASSERT ( vect, status, SFTUTILS_ENULL, SFTUTILS_MSGENULL );
  ASSERT ( *vect, status, SFTUTILS_ENONULL, SFTUTILS_MSGENONULL );

  for ( i=0; i < (*vect)->length; i++ )
    {
      if ( (*vect)->data[i] )
	LALFree ( (*vect)->data[i] );
    }

  LALFree ( (*vect)->data );
  LALFree ( (*vect) );

  (*vect) = NULL;

    RETURN(status);
} /* LALDestroyStringVector() */



/** Allocate a LIGOTimeGPSVector */
LIGOTimeGPSVector *
XLALCreateTimestampVector (UINT4 length)
{
  LIGOTimeGPSVector *out = NULL;

  out = LALCalloc (1, sizeof(LIGOTimeGPSVector));
  if (out == NULL) 
    XLAL_ERROR_NULL ( "XLALCreateTimestampVector", XLAL_ENOMEM );

  out->length = length;
  out->data = LALCalloc (1, length * sizeof(LIGOTimeGPS));
  if (out->data == NULL) {
    LALFree (out);
    XLAL_ERROR_NULL ( "XLALCreateTimestampVector", XLAL_ENOMEM );
  }

  return out;

} /* XLALCreateTimestampVector() */



/** LAL-interface: Allocate a LIGOTimeGPSVector */
void
LALCreateTimestampVector (LALStatus *status, 
			  LIGOTimeGPSVector **vect, 	/**< [out] allocated timestamp-vector  */
			  UINT4 length)			/**< number of elements */
{
  LIGOTimeGPSVector *out = NULL;

  INITSTATUS( status, "LALCreateTimestampVector", SFTUTILSC);

  ASSERT (vect != NULL, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);
  ASSERT (*vect == NULL, status, SFTUTILS_ENONULL,  SFTUTILS_MSGENONULL);

  if ( (out = XLALCreateTimestampVector( length )) == NULL ) {
    XLALClearErrno();
    ABORT (status,  SFTUTILS_EMEM,  SFTUTILS_MSGEMEM);
  }

  *vect = out;

  RETURN (status);
  
} /* LALCreateTimestampVector() */


/** De-allocate a LIGOTimeGPSVector */
void
XLALDestroyTimestampVector ( LIGOTimeGPSVector *vect)
{
  if ( !vect )
    return;

  LALFree ( vect->data );
  LALFree ( vect );
  
  return;
  
} /* XLALDestroyTimestampVector() */


/** De-allocate a LIGOTimeGPSVector
 */
void
LALDestroyTimestampVector (LALStatus *status, 
			   LIGOTimeGPSVector **vect)	/**< timestamps-vector to be freed */
{
  INITSTATUS( status, "LALDestroyTimestampVector", SFTUTILSC);

  ASSERT (vect != NULL, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);
  ASSERT (*vect != NULL, status, SFTUTILS_ENULL,  SFTUTILS_MSGENULL);

  XLALDestroyTimestampVector ( (*vect) );
  
  (*vect) = NULL;

  RETURN (status);
  
} /* LALDestroyTimestampVector() */



/** Given a start-time, duration and Tsft, returns a list of timestamps
 * covering this time-stretch.
 * returns NULL on out-of-memory
 */
void
LALMakeTimestamps(LALStatus *status,
		  LIGOTimeGPSVector **timestamps, 	/**< [out] timestamps-vector */
		  LIGOTimeGPS tStart, 			/**< GPS start-time */
		  REAL8 duration, 			/**< duration in seconds */
		  REAL8 Tsft)				/**< length of one (SFT) timestretch in seconds */
{
  UINT4 i;
  UINT4 numSFTs;
  LIGOTimeGPS tt;
  LIGOTimeGPSVector *ts = NULL;

  INITSTATUS( status, "LALMakeTimestamps", SFTUTILSC);
  ATTATCHSTATUSPTR (status);

  ASSERT (timestamps != NULL, status, SFTUTILS_ENULL, 
	  SFTUTILS_MSGENULL);
  ASSERT (*timestamps == NULL,status, SFTUTILS_ENONULL, 
	  SFTUTILS_MSGENONULL);

  ASSERT ( duration >= Tsft, status, SFTUTILS_EINPUT, SFTUTILS_MSGEINPUT);

  numSFTs = (UINT4)( duration / Tsft );			/* floor */
  if ( (ts = LALCalloc (1, sizeof( *ts )) ) == NULL ) {
    ABORT (status,  SFTUTILS_EMEM,  SFTUTILS_MSGEMEM);
  }

  ts->length = numSFTs;
  if ( (ts->data = LALCalloc (1, numSFTs * sizeof (*ts->data) )) == NULL) {
    ABORT (status,  SFTUTILS_EMEM,  SFTUTILS_MSGEMEM);
  }

  tt = tStart;	/* initialize to start-time */
  for (i = 0; i < numSFTs; i++)
    {
      ts->data[i] = tt;
      /* get next time-stamp */
      /* NOTE: we add the interval Tsft successively instead of
       * via iSFT*Tsft, because this way we avoid possible ns-rounding problems
       * with a REAL8 interval, which becomes critial from about 100days on...
       */
      LALAddFloatToGPS( status->statusPtr, &tt, &tt, Tsft);
      BEGINFAIL( status ) {
	LALFree (ts->data);
	LALFree (ts);
      } ENDFAIL(status);

    } /* for i < numSFTs */

  *timestamps = ts;

  DETATCHSTATUSPTR( status );
  RETURN( status );
  
} /* LALMakeTimestamps() */


/** Extract/construct the unique 2-character "channel prefix" from the given 
 * "detector-name", which unfortunately will not always follow any of the 
 * official detector-naming conventions given in the Frames-Spec LIGO-T970130-F-E
 * This function therefore sometime has to do some creative guessing:
 *
 * NOTE: in case the channel-number can not be deduced from the name, 
 * it is set to '1', and a warning will be printed if lalDebugLevel > 0.
 *
 * NOTE2: the returned string is allocated here!
 */
CHAR *
XLALGetChannelPrefix ( const CHAR *name )
{
  CHAR *channel = LALCalloc( 3, sizeof(CHAR) );  /* 2 chars + \0 */

  if ( !channel ) {
    XLAL_ERROR_NULL ( "XLALGetChannelPrefix", XLAL_ENOMEM );
  }
  if ( !name ) {
    LALFree ( channel );
    XLAL_ERROR_NULL ( "XLALGetChannelPrefix", XLAL_EINVAL );
  }

  /* first handle (currently) unambiguous ones */
  if ( strstr( name, "ALLEGRO") || strstr ( name, "A1") )
    strcpy ( channel, "A1");
  else if ( strstr(name, "NIOBE") || strstr( name, "B1") )
    strcpy ( channel, "B1");
  else if ( strstr(name, "EXPLORER") || strstr( name, "E1") )
    strcpy ( channel, "E1");
  else if ( strstr(name, "GEO") || strstr(name, "G1") )
    strcpy ( channel, "G1" );
  else if ( strstr(name, "ACIGA") || strstr (name, "K1") )
    strcpy ( channel, "K1" );
  else if ( strstr(name, "LLO") || strstr(name, "Livingston") || strstr(name, "L1") )
    strcpy ( channel, "L1" );
  else if ( strstr(name, "Nautilus") || strstr(name, "N1") )
    strcpy ( channel, "N1" );
  else if ( strstr(name, "AURIGA") || strstr(name,"O1") )
    strcpy ( channel, "O1" );
  else if ( strstr(name, "CIT_40") || strstr(name, "Caltech-40") || strstr(name, "P1") )
    strcpy ( channel, "P1" );
  else if ( strstr(name, "TAMA") || strstr(name, "T1") )
    strcpy (channel, "T1" );
  else if ( strstr(name, "Virgo") || strstr(name, "V1") || strstr(name, "V2") )
    {
      if ( strstr(name, "Virgo_CITF") || strstr(name, "V2") )
	strcpy ( channel, "V2" );
      else if ( strstr(name, "Virgo") || strstr(name, "V1") )
	strcpy ( channel, "V1" );
    } /* if Virgo */
  /* currently the only real ambiguity arises with H1 vs H2 */
  else if ( strstr(name, "LHO") || strstr(name, "Hanford") || strstr(name, "H1") || strstr(name, "H2") )
    {
      if ( strstr(name, "LHO_2k") ||  strstr(name, "H2") )
	strcpy ( channel, "H2" );
      else if ( strstr(name, "LHO_4k") ||  strstr(name, "H1") )
	strcpy ( channel, "H1" );
      else /* otherwise: guess */
	{
	  strcpy ( channel, "H1" );
	  if ( lalDebugLevel ) 
	    LALPrintError("WARNING: Detector-name '%s' not unique, guessing '%s'\n", name, channel );
	} 
    } /* if LHO */
    
  if ( channel[0] == 0 )
    {
      if ( lalDebugLevel ) LALPrintError ( "\nERROR: unknown detector-name '%s'\n\n", name );
      XLAL_ERROR_NULL ( "XLALGetChannelPrefix", XLAL_EINVAL );
    }
  else
    return channel;

} /* XLALGetChannelPrefix() */


/** Find the site geometry-information 'LALDetector' (mis-nomer!) given a detector-name.
 * The LALDetector struct is allocated here.
 */
LALDetector * 
XLALGetSiteInfo ( const CHAR *name )
{
  CHAR *channel;
  LALDetector *site;

  /* first turn the free-form 'detector-name' into a well-defined channel-prefix */
  if ( ( channel = XLALGetChannelPrefix ( name ) ) == NULL ) {
    XLAL_ERROR_NULL ( "XLALGetSiteInfo()", XLAL_EINVAL );
  }

  if ( ( site = LALCalloc ( 1, sizeof( *site) )) == NULL ) {
    XLAL_ERROR_NULL ( "XLALGetSiteInfo()", XLAL_ENOMEM );
  }
  
  switch ( channel[0] )
    {
    case 'T':
      (*site) = lalCachedDetectors[LALDetectorIndexTAMA300DIFF];
      break;
    case 'V':
      (*site) = lalCachedDetectors[LALDetectorIndexVIRGODIFF];
      break;
    case 'G':
      (*site) = lalCachedDetectors[LALDetectorIndexGEO600DIFF];
      break;
    case 'H':
      (*site) = lalCachedDetectors[LALDetectorIndexLHODIFF];
      break;
    case 'L':
      (*site) = lalCachedDetectors[LALDetectorIndexLLODIFF];
      break;
    case 'P':
      (*site) = lalCachedDetectors[LALDetectorIndexCIT40DIFF];
      break;
    case 'N':
      if ( 0 != create_nautilus_site ( site ) ) 
	{
	  if ( lalDebugLevel ) LALPrintError("\nFailed to created Nautilus detector-site info\n\n");
	  LALFree ( site );
	  LALFree ( channel );
	  XLAL_ERROR_NULL ( "XLALGetSiteInfo()", XLAL_EFUNC );
	}
      break;

    default:
      if ( lalDebugLevel ) 
	LALPrintError ( "\nSorry, I don't have the site-info for '%c%c'\n\n", channel[0], channel[1]);
      LALFree(site);
      LALFree(channel);
      XLAL_ERROR_NULL ( "XLALGetSiteInfo()", XLAL_EINVAL );
      break;
    } /* switch channel[0] */

  /* "hack" the returned LALDetector-structure, to contain a proper detector-unique 'name'-entry */
  strcpy ( site->frDetector.name , channel );

  LALFree ( channel );

  return site;
  
} /* XLALGetSiteInfo() */


/* Set up the \em LALDetector struct representing the NAUTILUS detector-site 
 * return -1 on ERROR, 0 if OK
 */
static int 
create_nautilus_site ( LALDetector *Detector )
{
  LALFrDetector detector_params;
  LALDetector Detector1;
  LALStatus status = empty_status;

  if ( !Detector )
    return -1;

  strcpy ( detector_params.name, "NAUTILUS" );
  detector_params.vertexLongitudeRadians = 12.67 * LAL_PI / 180.0;
  detector_params.vertexLatitudeRadians = 41.82 * LAL_PI / 180.0;
  detector_params.vertexElevation = 300.0;
  detector_params.xArmAltitudeRadians = 0.0;
  detector_params.xArmAzimuthRadians = 44.0 * LAL_PI / 180.0;

  LALCreateDetector(&status, &Detector1, &detector_params, LALDETECTORTYPE_CYLBAR );
  if ( status.statusCode != 0 )
    return -1;

  (*Detector) = Detector1;

  return 0;
  
} /* CreateNautilusDetector() */


