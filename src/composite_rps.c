/****************************************************************
 * file composite_rps.c
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It supports composite values.
 *
 *
 *      © Copyright 2019 - 2021 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "Refpersys.h"

#include "kavl.h"

const RpsTupleOb_t *
rps_alloc_tuple_sized (unsigned arity, RpsObject_t ** arr)
{
  RpsTupleOb_t *tup = NULL;
  RpsHash_t htup = 0;
  unsigned long h1 = 0, h2 = rps_prime_above (3 * arity + 5);
  if (arr == NULL && arity > 0)
    return NULL;
  tup =
    RPS_ALLOC_ZONE (sizeof (RpsTupleOb_t) + arity * sizeof (RpsObject_t *),
		    RPS_TYPE_TUPLE);
  for (int ix = 0; ix < (int) arity; ix++)
    {
      RpsObject_t *curob = arr[ix];
      if (curob && rps_is_valid_object (curob))
	{
	  RpsHash_t curhash = curob->zv_hash;
	  tup->tuple_comp[ix] = (RpsObject_t *) curob;
	  if (ix % 2 == 0)
	    {
	      unsigned oldh1 = h1;
	      h1 = ((32059 * h1) ^ (curhash * 32083)) + ix;
	      h2 = ((oldh1 << 11) ^ curhash) + ((h2 >> 17) * 321073);
	    }
	  else
	    {
	      unsigned oldh2 = h2;
	      h1 = (32009 * h1) ^ ((curhash * 52069) + oldh2 - ix);
	      h2 = (oldh2 % 152063) ^ ((curhash << 5) + (541 * h2));
	    }
	}
    }
  htup = h1 ^ h2;
  if (!htup)
    htup = rps_prime_above ((h1 & 0xfffff) + (h2 & 0xffffff));
  tup->zv_hash = htup;
  tup->zm_length = arity;
  return tup;
}				/* end of rps_alloc_tuple_sized */


const RpsTupleOb_t *
rps_alloc_vtuple (unsigned arity, ...)
{
  va_list arglist;
  const RpsTupleOb_t *tup = NULL;
  RpsObject_t **obarr =
    RPS_ALLOC_ZEROED ((arity + 1) * sizeof (RpsObject_t *));
  va_start (arglist, arity);
  for (int ix = 0; ix < (int) arity; ix++)
    {
      obarr[ix] = va_arg (arglist, RpsObject_t *);
    }
  va_end (arglist);
  tup = rps_alloc_tuple_sized (arity, obarr);
  free (obarr);
  return tup;
}				/* end rps_alloc_vtuple */

unsigned
rps_vtuple_size (const RpsTupleOb_t * tup)
{
  if (tup == NULL || rps_value_type ((RpsValue_t) tup) != RPS_TYPE_TUPLE)
    return 0;
  return tup->zm_length;
}				/* end rps_vtuple_size */

RpsObject_t *
rps_vtuple_nth (const RpsTupleOb_t * tup, int rk)
{
  if (tup == NULL || rps_value_type ((RpsValue_t) tup) != RPS_TYPE_TUPLE)
    return NULL;
  unsigned sz = tup->zm_length;
  if (rk < 0)
    rk += (int) sz;
  if (rk >= 0 && rk < sz)
    return tup->tuple_comp[rk];
  return NULL;
}				/* end rps_vtuple_nth */





const RpsSetOb_t *
rps_alloc_set_sized (unsigned nbcomp, const RpsObject_t ** arr)
{
  RpsSetOb_t *set = NULL;
  if (!arr && nbcomp > 0)
    return NULL;
  const RpsObject_t **arrcpy =
    RPS_ALLOC_ZEROED ((nbcomp + 1) * sizeof (RpsObject_t *));
  memcpy (arrcpy, arr, nbcomp * sizeof (RpsObject_t *));
  rps_object_array_qsort (arrcpy, (int) nbcomp);
  int card = 0;
  for (int ix = 0; ix < (int) nbcomp - 1; ix++)
    if (arrcpy[ix + 1] != arrcpy[ix] && arrcpy[ix])
      card++;
  set =
    RPS_ALLOC_ZONE (sizeof (RpsSetOb_t) + card * sizeof (RpsObject_t *),
		    RPS_TYPE_SET);
  set->zm_length = card;
  int nbel = 0;
  for (int ix = 0; ix < (int) nbcomp - 1; ix++)
    if (arrcpy[ix + 1] != arrcpy[ix] && arrcpy[ix])
      set->set_elem[nbel++] = arrcpy[ix];
  if (set->set_elem[nbel] != arrcpy[nbcomp - 1] && arrcpy[nbcomp - 1])
    set->set_elem[nbel++] = arrcpy[nbcomp - 1];
  free (arrcpy);
  RPS_ASSERT (card == nbel);
  return set;
}				/* end rps_alloc_set_sized */

const RpsSetOb_t *
rps_alloc_vset (unsigned card, /*objects */ ...)
{
  const RpsSetOb_t *set = NULL;
  va_list arglist;
  va_start (arglist, card);
  RpsObject_t **arrcpy =
    RPS_ALLOC_ZEROED ((card + 1) * sizeof (RpsObject_t *));
  for (int ix = 0; ix < (int) card; ix++)
    {
      arrcpy[ix] = va_arg (arglist, RpsObject_t *);
    }
  va_end (arglist);
  set = rps_alloc_set_sized (card, (const RpsObject_t **) arrcpy);
  free (arrcpy);
  return set;
}				/* end rps_alloc_vset */

const RpsClosure_t *
rps_closure_array_make (RpsObject_t * conn, RpsValue_t meta, unsigned arity,
			RpsValue_t * cvalarr)
{
  RpsClosure_t *clos = NULL;
  if (conn == NULL || !rps_is_valid_object (conn))
    return NULL;
  RPS_ASSERT (arity == 0 || cvalarr != NULL);
  RPS_ASSERT (arity < RPS_CLOSURE_MAX_NB_VALUE);
  unsigned size = rps_prime_above (arity);
  int prix = rps_index_of_prime (size);
  clos =
    RPS_ALLOC_ZONE (sizeof (RpsClosure_t) + size * sizeof (RpsValue_t),
		    RPS_TYPE_SET);
  clos->zm_length = arity;
  clos->zm_xtra = prix;
  clos->clos_conn = conn;
  clos->clos_meta = meta;
  memcpy (clos->clos_val, cvalarr, arity * sizeof (RpsValue_t));
  return clos;
}				/* end rps_closure_array_make */

#define RPS_SMALL_SIZE 64
const RpsClosure_t *
rps_closure_make (RpsObject_t * conn, unsigned arity, ...)
{
  va_list arglist;
  const RpsClosure_t *clos = NULL;
  if (!conn)
    return NULL;
  va_start (arglist, arity);
  RPS_ASSERT (arity < RPS_CLOSURE_MAX_NB_VALUE);
  RpsValue_t *arr = NULL;
  if (arity < RPS_SMALL_SIZE)
    {
      RpsValue_t smallarr[RPS_SMALL_SIZE];
      memset (smallarr, 0, sizeof (smallarr));
      for (int ix = 0; ix < (int) arity; ix++)
	smallarr[ix] = va_arg (arglist, RpsValue_t);
      clos = rps_closure_array_make (conn, arity, RPS_NULL_VALUE, smallarr);
    }
  else
    {
      arr = RPS_ALLOC_ZEROED (sizeof (RpsValue_t) * (arity + 1));
      for (int ix = 0; ix < (int) arity; ix++)
	arr[ix] = va_arg (arglist, RpsValue_t);
      clos = rps_closure_array_make (conn, arity, RPS_NULL_VALUE, arr);
      free (arr);
    }
  va_end (arglist);
  return clos;
}				/* end rps_closure_array_make */

const RpsClosure_t *
rps_closure_meta_make (RpsObject_t * conn, RpsValue_t meta, unsigned arity,
		       ...)
{
  va_list arglist;
  const RpsClosure_t *clos = NULL;
  if (!conn)
    return NULL;
  va_start (arglist, arity);
  RPS_ASSERT (arity < RPS_CLOSURE_MAX_NB_VALUE);
  RpsValue_t *arr = NULL;
  if (arity < RPS_SMALL_SIZE)
    {
      RpsValue_t smallarr[RPS_SMALL_SIZE];
      memset (smallarr, 0, sizeof (smallarr));
      for (int ix = 0; ix < (int) arity; ix++)
	smallarr[ix] = va_arg (arglist, RpsValue_t);
      clos = rps_closure_array_make (conn, arity, RPS_NULL_VALUE, smallarr);
    }
  else
    {
      arr = RPS_ALLOC_ZEROED (sizeof (RpsValue_t) * (arity + 1));
      for (int ix = 0; ix < (int) arity; ix++)
	arr[ix] = va_arg (arglist, RpsValue_t);
      clos = rps_closure_array_make (conn, arity, RPS_NULL_VALUE, arr);
      free (arr);
    }
  va_end (arglist);
  return clos;
}				/* end rps_closure_meta_make */





/**** mutable set payload support *****/

static int
rps_mutable_set_ob_node_cmp (const struct internal_mutable_set_ob_node_rps_st
			     *left,
			     const struct internal_mutable_set_ob_node_rps_st
			     *right)
{
  RPS_ASSERT (left);
  RPS_ASSERT (right);
  RpsObject_t *obleft = left->setobnodrps_obelem;
  RpsObject_t *obright = right->setobnodrps_obelem;
  if (obleft == obright)
    return 0;
  if (obleft == NULL)
    return -1;
  if (obright == NULL)
    return 1;
  RPS_ASSERT (obleft != NULL && obleft->zm_type == RPS_TYPE_OBJECT);
  RPS_ASSERT (obright != NULL && obright->zm_type == RPS_TYPE_OBJECT);
  return rps_oid_cmp (obleft->ob_id, obright->ob_id);
}				/* end rps_mutable_set_ob_node_cmp */

KAVL_INIT (rpsmusetob, struct internal_mutable_set_ob_node_rps_st,
	   setobnodrps_head, rps_mutable_set_ob_node_cmp);


/* This function returns true if `ob` was genuinely added into
   `paylmset`, and false otherwise, e.g. because it was already
   element of the mutable set `paylmset`. */
bool
rps_paylsetob_add_element (RpsMutableSetOb_t * paylmset,
			   const RpsObject_t * ob)
{
  RPS_ASSERT (paylmset != NULL && paylmset->zm_type == -RpsPyt_MutableSetOb);
  RPS_ASSERT (ob != NULL && rps_is_valid_object (ob));
  struct internal_mutable_set_ob_node_rps_st *newnod =
    RPS_ALLOC_ZEROED (sizeof (struct internal_mutable_set_ob_node_rps_st));
  newnod->setobnodrps_obelem = ob;
  struct internal_mutable_set_ob_node_rps_st *addednod =
    kavl_insert_rpsmusetob (&paylmset->muset_root, newnod, NULL);
  if (addednod == newnod)
    {
      paylmset->zm_length++;
      return true;
    }
  else
    {
      free (newnod);
      return false;
    }
}				/* end rps_paylsetob_add_element */


/* This function returns true if `ob` was genuinely removed into
   `paylmset`, and false otherwise, e.g. because it was not an element
   of the mutable set `paylmset`. */
bool
rps_paylsetob_remove_element (RpsMutableSetOb_t * paylmset,
			      const RpsObject_t * ob)
{
  RPS_ASSERT (paylmset != NULL && paylmset->zm_type == -RpsPyt_MutableSetOb);
  RPS_ASSERT (ob != NULL && rps_is_valid_object ((RpsObject_t *) ob));
  struct internal_mutable_set_ob_node_rps_st *newnod =
    RPS_ALLOC_ZEROED (sizeof (struct internal_mutable_set_ob_node_rps_st));
  newnod->setobnodrps_obelem = ob;
  struct internal_mutable_set_ob_node_rps_st *removednod =
    kavl_erase_rpsmusetob(&paylmset->muset_root, newnod, NULL);
  if (removednod) {
    free (removednod);
    free (newnod);
    paylmset->zm_length--;
    return true;
  }
  else {
    free (newnod);
    return false;
  };
}				/* end rps_paylsetob_remove_element */


/* loading mutable set of objects */
void
rpsldpy_setob (RpsObject_t * obj, RpsLoader_t * ld, const json_t * jv,
	       int spix)
{
  RpsMutableSetOb_t *paylsetob = NULL;
  RPS_ASSERT (obj != NULL);
  RPS_ASSERT (rps_is_valid_filling_loader (ld));
  paylsetob = RPS_ALLOC_ZONE (sizeof (RpsMutableSetOb_t),
			      -RpsPyt_MutableSetOb);
  json_t *jssetob = json_object_get (jv, "setob");
  if (jssetob && json_is_array (jssetob))
    {
      int card = (int) json_array_size (jssetob);
      for (int ix = 0; ix < card; ix++)
	{
	  json_t *jcurelem = json_array_get (jssetob, ix);
	  RpsObject_t *elemob = rps_loader_json_to_object (ld, jcurelem);
	  RPS_ASSERT (elemob != NULL);
	  if (!rps_paylsetob_add_element (paylsetob, elemob))
	    RPS_FATAL ("corrupted already element element#%d for json %s",
		       ix, json_dumps (jv, JSON_INDENT (2) | JSON_SORT_KEYS));
	};
    }
  rps_object_put_payload (obj, paylsetob);
}				/* end rpsldpy_setob */


void
rps_object_mutable_set_initialize (RpsObject_t * obj)
{
  RpsMutableSetOb_t *paylsetob = NULL;
  RPS_ASSERT (obj != NULL);
  RPS_ASSERT (rps_is_valid_object (obj));
  pthread_mutex_lock (&obj->ob_mtx);
  paylsetob = RPS_ALLOC_ZONE (sizeof (RpsMutableSetOb_t),
			      -RpsPyt_MutableSetOb);
  rps_object_put_payload (obj, paylsetob);
  pthread_mutex_unlock (&obj->ob_mtx);
}				/* end rps_object_mutable_set_initialize */

void
rps_object_mutable_set_add (RpsObject_t * obj, RpsValue_t val)
{
  RpsMutableSetOb_t *paylsetob = NULL;
  RPS_ASSERT (obj != NULL);
  RPS_ASSERT (rps_is_valid_object (obj));
  if (!val || val == RPS_NULL_VALUE || (val & 1))
    return;
  enum RpsType vtyp = rps_value_type (val);
  pthread_mutex_lock (&obj->ob_mtx);
  if (!obj->ob_payload)
    goto end;
  if (((RpsMutableSetOb_t *) (obj->ob_payload))->zm_type ==
      -RpsPyt_MutableSetOb)
    paylsetob = (RpsMutableSetOb_t *) (obj->ob_payload);
  else
    goto end;
  switch (vtyp)
    {
    case RPS_TYPE_TUPLE:
      {
	const RpsTupleOb_t *tup = (const RpsTupleOb_t *) val;
	unsigned sz = tup->zm_length;
	for (int ix = 0; ix < (int) sz; ix++)
	  if (tup->tuple_comp[ix] != NULL)
	    (void) rps_paylsetob_add_element (paylsetob, tup->tuple_comp[ix]);
      }
      break;
    case RPS_TYPE_SET:
      {
	const RpsSetOb_t *set = (const RpsSetOb_t *) val;
	unsigned sz = set->zm_length;
	for (int ix = 0; ix < (int) sz; ix++)
	  (void) rps_paylsetob_add_element (paylsetob, set->set_elem[ix]);
      }
      break;
    case RPS_TYPE_OBJECT:
      (void) rps_paylsetob_add_element (paylsetob, (RpsObject_t *) val);
      break;
    default:
      goto end;
    }
end:
  pthread_mutex_unlock (&obj->ob_mtx);
}				/* end rps_object_mutable_set_add */





/* loading a payload associating strings to values */
void
rpsldpy_string_dictionary (RpsObject_t * obj, RpsLoader_t * ld,
			   const json_t * jv, int spix)
{
  RPS_ASSERT (obj != NULL);
  RPS_ASSERT (rps_is_valid_filling_loader (ld));
  char idbuf[32];
  memset (idbuf, 0, sizeof (idbuf));
  rps_oid_to_cbuf (obj->ob_id, idbuf);
#warning rpsldpy_string_dictionary unimplemented
  RPS_FATAL ("unimplemented rpsldpy_string_dictionary %s spix#%d jv...\n%s\n",
	     idbuf, spix, json_dumps (jv, JSON_INDENT (2) | JSON_SORT_KEYS));
}				/* end rpsldpy_string_dictionary */

/***************** end of file composite_rps.c from refpersys.org **********/
