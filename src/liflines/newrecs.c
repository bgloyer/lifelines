/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * newrecs.c -- Handle source, event and other record types
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 11 Sep 94    3.0.2 - 14 Apr 95
 *   3.0.3 - 17 Feb 96
 *===========================================================*/

#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"
#include "lloptions.h"

#include "llinesi.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern BTREE BTR;
extern STRING qScfradd, qScfeadd, qScfxadd, qSrredit, qSeredit, qSxredit;
extern STRING qScfrupt, qScfeupt, qScfxupt, qSgdrmod, qSgdemod, qSgdxmod;
extern STRING qSidredt, qSideedt, qSidxedt, qSduprfn, qSronlya, qSronlye;
extern STRING qSrreditopt, qSereditopt, qSxreditopt;
extern STRING qSnofopn, qSidkyrfn;
extern STRING qSdefsour,qSdefeven,qSdefothr,qSnosuchrec;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static RECORD edit_add_record(STRING recstr, STRING redt, STRING redtopt
	, char ntype, STRING cfrm);
static BOOLEAN edit_record(RECORD rec1, STRING idedt, INT letr, STRING redt
	, STRING redtopt , BOOLEAN (*val)(NODE, STRING *, NODE), STRING cfrm
	, void (*todbase)(NODE), STRING gdmsg);


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*================================================
 * edit_add_source -- Add source to database by editing
 *==============================================*/
RECORD
edit_add_source (void)
{
	STRING str;
	if (readonly) {
		message(_(qSronlya));
		return NULL;
	}
	str = getoptstr("SOURREC", _(qSdefsour));
	return edit_add_record(str, _(qSrredit), _(qSrreditopt), 'S', _(qScfradd));
}
/*==============================================
 * edit_add_event -- Add event to database by editing
 *============================================*/
RECORD
edit_add_event (void)
{
	STRING str;
	if (readonly) {
		message(_(qSronlya));
		return NULL;
	}
	str = getoptstr("EVENREC", _(qSdefeven));
	return edit_add_record(str, _(qSeredit), _(qSereditopt), 'E', _(qScfeadd));
}
/*====================================================
 * edit_add_other -- Add user record to database by editing
 *==================================================*/
RECORD
edit_add_other (void)
{
	STRING str;
	if (readonly) {
		message(_(qSronlya));
		return NULL;
	}
	str = getoptstr("OTHR", _(qSdefothr));
	return edit_add_record(str, _(qSxredit), _(qSxreditopt), 'X', _(qScfxadd));
}
/*================================================
 * edit_add_record -- Add record to database by editing
 *  recstr:  [IN] default record
 *  redt:    [IN] re-edit message
 *  ntype,   [IN] S, E, or X
 *  cfrm:    [IN] confirm message
 *==============================================*/
static RECORD
edit_add_record (STRING recstr, STRING redt, STRING redtopt, char ntype, STRING cfrm)
{
	FILE *fp;
	NODE node=0, refn;
	STRING msg, key;
	BOOLEAN emp;
	XLAT ttmi = get_tranmapping(MEDIN);
	STRING (*getreffnc)(void) = NULL; /* get next internal key */
	void (*todbasefnc)(NODE) = NULL;  /* write record to dbase */
	void (*tocachefnc)(NODE) = NULL;  /* write record to cache */
	
	/* set up functions according to type */
	if (ntype == 'S') {
		getreffnc = getsxref;
		todbasefnc = sour_to_dbase;
		tocachefnc = sour_to_cache;
	} else if (ntype == 'E') {
		getreffnc = getexref;
		todbasefnc = even_to_dbase;
		tocachefnc = even_to_cache;
	} else { /* X */
		getreffnc = getxxref;
		todbasefnc = othr_to_dbase;
		tocachefnc = othr_to_cache;
	}

/* Create template for user to edit */
	if (!(fp = fopen(editfile, LLWRITETEXT))) {
		msg_error(_(qSnofopn), editfile);
		return FALSE;
	}
	fprintf(fp, "%s\n", recstr);

/* Have user edit new record */
	fclose(fp);
	do_edit();
	while (TRUE) {
		INT cnt;
		node = file_to_node(editfile, ttmi, &msg, &emp);
		if (!node) {
			if (ask_yes_or_no_msg(msg, redt)) { /* yes, edit again */
				do_edit();
				continue;
			} 
			break;
		}
		cnt = resolve_refn_links(node);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!valid_node_type(node, ntype, &msg, NULL)) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			free_nodes(node);
			node = NULL; /* fail out */
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			snprintf(msgb, sizeof(msgb)
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, redtopt)) {
				write_node_to_editfile(node);
				do_edit();
				continue;
			}
		}
		break;
	}
	if (!node || !ask_yes_or_no(cfrm)) {
		if (node) free_nodes(node);
		return NULL;
	}
	nxref(node) = strsave((STRING)(*getreffnc)());
	key = rmvat(nxref(node));
	for (refn = nchild(node); refn; refn = nsibling(refn)) {
		if (eqstr("REFN", ntag(refn)) && nval(refn))
			add_refn(nval(refn), key);
	}
	(*todbasefnc)(node);
	(*tocachefnc)(node);
	return key_to_record(key);
}
/*=======================================
 * edit_source -- Edit source in database
 *=====================================*/
BOOLEAN
edit_source (RECORD rec)
{
	return edit_record(rec, _(qSidredt), 'S', _(qSrredit), _(qSrreditopt)
		, valid_sour_tree, _(qScfrupt), sour_to_dbase, _(qSgdrmod));
}
/*=====================================
 * edit_event -- Edit event in database
 *===================================*/
BOOLEAN
edit_event (RECORD rec)
{
	return edit_record(rec, _(qSideedt), 'E', _(qSeredit), _(qSereditopt)
		, valid_even_tree, _(qScfeupt), even_to_dbase, _(qSgdemod));
}
/*===========================================
 * edit_other -- Edit other record in database (eg, NOTE)
 *=========================================*/
BOOLEAN
edit_other (RECORD rec)
{
	return edit_record(rec, _(qSidxedt), 'X', _(qSxredit), _(qSxreditopt)
		, valid_othr_tree, _(qScfxupt), othr_to_dbase, _(qSgdxmod));
}
/*=======================================
 * edit_any_record -- Edit record of any type
 *=====================================*/
BOOLEAN
edit_any_record (RECORD rec)
{
	ASSERT(rec);
	switch (nztype(rec)) {
	case 'I': return edit_indi(rec);
	case 'F': return edit_family(rec);
	case 'S': return edit_source(rec);
	case 'E': return edit_event(rec);
	case 'X': return edit_other(rec);
	default: ASSERT(0); return FALSE;
	}
}
/*========================================================
 * write_node_to_editfile - write all parts of gedcom node
 *  to a file for editing
 *======================================================*/
void
write_node_to_editfile (NODE node)
{
	FILE *fp;
	XLAT ttmo = get_tranmapping(MINED);

	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, ttmo, node,  TRUE, TRUE, TRUE);
	fclose(fp);
}
/*=======================================
 * edit_record -- Edit record in database
 *  root1:   [IN]  record to edit (may be NULL)
 *  idedt:   [IN]  user id prompt
 *  letr:    [IN]  record type (E, S, or X)
 *  redt:    [IN]  reedit prompt displayed if hard error after editing
 *  redtopt: [IN]  reedit prompt displayed if soft error (unresolved links)
 *  val:     [IN]  callback to validate routine
 *  cfrm:    [IN]  confirmation msg string
 *  tag:     [IN]  tag (SOUR, EVEN, or NULL)
 *  todbase: [IN]  callback to write record to dbase
 *  gdmsg:   [IN]  success message
 *=====================================*/
static BOOLEAN
edit_record (RECORD rec1, STRING idedt, INT letr, STRING redt, STRING redtopt
	, BOOLEAN (*val)(NODE, STRING *, NODE)
	, STRING cfrm, void (*todbase)(NODE), STRING gdmsg)
{
	XLAT ttmi = get_tranmapping(MEDIN);
	STRING msg, key;
	BOOLEAN emp;
	NODE root0=0, root1=0, root2=0;
	NODE refn1=0, refn2=0, refnn=0, refn1n=0;
	NODE body=0, node=0;

/* Identify record if need be */
	if (!rec1) {
		rec1 = ask_for_record(idedt, letr);
	}
	root1 = nztop(rec1);
	if (!root1) {
		message(_(qSnosuchrec));
		return FALSE;
	}

/* Have user edit record */
	if (getoptint("ExpandRefnsDuringEdit", 0) > 0)
		expand_refn_links(root1);
	write_node_to_editfile(root1);
	resolve_refn_links(root1);

	do_edit();
	if (readonly) {
		root2 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!equal_tree(root1, root2))
			message(_(qSronlye));
		free_nodes(root2);
		return FALSE;
	}

	while (TRUE) {
		INT cnt;
		root2 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!root2) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			break;
		}
		cnt = resolve_refn_links(root2);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!(*val)(root2, &msg, root1)) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			free_nodes(root2);
			root2 = NULL;
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			snprintf(msgb, sizeof(msgb)
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, redtopt)) {
				write_node_to_editfile(root2);
				do_edit();
				continue;
			}
		}
		break;
	}

/* If error or no change or user backs out return */
	if (!root2) return FALSE;
	if (equal_tree(root1, root2) || !ask_yes_or_no(cfrm)) {
		free_nodes(root2);
		return FALSE;
	}

/* Prepare to change database */

	/* Move root1 data into root0 & save refns */
	split_othr(root1, &refn1, &body);
	root0 = copy_node(root1);
	join_othr(root0, NULL, body);
	/* delete root0 tree & root1 node (root1 is solitary node) */
	free_nodes(root0); root0 = 0;
	free_nodes(root1); root1 = 0;
	/* now copy root2 node into root1, then root2 tree under it */
	root1 = copy_node(root2);
	split_othr(root2, &refn2, &body);
	refnn = copy_nodes(refn2, TRUE, TRUE);
	join_othr(root1, refn2, body);
	/* now root2 is solitary node, delete it */
	free_node(root2); root2 = 0;

/* Change the database */

	(*todbase)(root1);
	key = rmvat(nxref(root1));
	/* remove deleted refns & add new ones */
	classify_nodes(&refn1, &refnn, &refn1n);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	free_nodes(refn1);
	free_nodes(refnn);
	free_nodes(refn1n);
	msg_info(gdmsg);
	return TRUE;
}
/*===============================================
 * ask_for_record -- Ask user to identify record
 *  lookup by key or by refn (& handle dup refns)
 *  idstr: [IN]  question prompt
 *  letr:  [IN]  letter to possibly prepend to key (ie, I/F/S/E/X)
 *=============================================*/
RECORD
ask_for_record (STRING idstr, INT letr)
{
	RECORD rec;
	char answer[MAXPATHLEN];
	if (!ask_for_string(idstr, _(qSidkyrfn), answer, sizeof(answer))
		|| !answer[0])
		return NULL;

	rec = key_possible_to_record(answer, letr);
	if (!rec) {
		INDISEQ seq;
		seq = refn_to_indiseq(answer, letr, KEYSORT);
		if (!seq) return NULL;
		rec = choose_from_indiseq(seq, NOASK1, _(qSduprfn), _(qSduprfn));
		remove_indiseq(seq);
	}
	return rec;
}
