/*-------------------------------------------------------------------------
 *
 * nodes.c
 *	  support code for nodes (now that we have removed the home-brew
 *	  inheritance system, our support code for nodes is much simpler)
 *
 * Portions Copyright (c) 2003-2013, PgPool Global Development Group
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/nodes/nodes.c
 *
 * HISTORY
 *	  Andrew Yu			Oct 20, 1994	file creation
 *
 *-------------------------------------------------------------------------
 */
/*#include "postgres.h"*/

#include "../pool_type.h"
#include "nodes.h"

/*
 * Support for newNode() macro
 */

Node	   *newNodeMacroHolder;
