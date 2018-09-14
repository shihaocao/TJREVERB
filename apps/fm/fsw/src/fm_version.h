/*
** $Id: fm_version.h 1.1.3.2.2.5 2012/06/05 15:10:47EDT nschweis Exp  $
**
** Title: File Manager (FM) Version Information Header File
**
** Purpose: Specification for the CFS FM application version label
**          definitions
**
** Author: Susanne L. Strege, Code 582 NASA GSFC
**
** Notes:
**
** References:
**    Flight Software Branch C Coding Standard Version 1.0a
**    CFS Flight Software Development Standards Version 0.11
**
** $Log: fm_version.h  $
** Revision 1.1.3.2.2.5 2012/06/05 15:10:47EDT nschweis 
** Changing the version number for the branch to 2.3.1.
** Revision 1.1.3.2.2.4 2012/05/08 16:20:09EDT nschweis 
** Changing the version number for the branch to 9.9.9.
** Revision 1.1.3.2.2.3 2012/05/08 16:05:53EDT nschweis 
** Changing the version number for the branch to 2.3.1.
** Revision 1.1.3.2.2.2 2012/03/07 13:27:20EST nschweis 
** Changing the version number for the branch to 9.9.9.
** Revision 1.1.3.2.2.1 2012/03/07 13:21:03EST nschweis 
** Changing the version number for the branch to 2.3.0.
** Revision 1.1.3.2 2011/01/12 14:37:50EST lwalling 
** Move mission revision number to platform config header file
** Revision 1.1.3.1 2009/10/30 14:02:28EDT lwalling 
** Remove trailing white space from all lines
** Revision 1.1 2008/10/03 15:35:16EDT sstrege
** Initial revision
** Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/fm/fsw/src/project.pj
*/

#ifndef _fm_version_h_
#define _fm_version_h_

/*************************************************************************
**
** Macro definitions
**
**************************************************************************/

/*
**  Application Version Information:
**
**  Major.Minor.Revision.Mission_Rev
**
**  Major: Major update.  This would include major changes or new functionality.
**         Most likely will include database schema changes and interface changes.
**         Probably not backwards compatible with older versions
**
**	Minor: Minor change, may introduce new features, but backwards compatibility is mostly
**         retained.  Likely will include schema changes.
**
**  Revision: Minor bug fixes, no significant new features implemented, though a few small
**            improvements may be included.  May include a schema change.
**
**  Mission_Rev:  Used by users of the applications (nominally missions) to denote changes made
**                by the mission.  Releases from the Flight Softare Reuse Library (FSRL) should
**                use Mission_Rev zero (0).
**
*/
#define FM_MAJOR_VERSION     2
#define FM_MINOR_VERSION     3
#define FM_REVISION          1

#endif /* _fm_version_h_ */

/************************/
/*  End of File Comment */
/************************/

