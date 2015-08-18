#include <u.h>
#include <ds/ds.h>
#include <gc/gc.h>
#include "c.h"

int
convrank(CTy *t)
{
	if(t->t != CPRIM)
		panic("internal error");
	switch(t->Prim.type){
	case PRIMCHAR:
		return 0;
	case PRIMSHORT:
		return 1;
	case PRIMINT:
		return 2;
	case PRIMLONG:
		return 3;
	case PRIMLLONG:
		return 4;
	case PRIMFLOAT:
		return 5;
	case PRIMDOUBLE:
		return 6;
	case PRIMLDOUBLE:
		return 7;
	}
	panic("internal error");
	return -1;
}

int 
compatiblestruct(CTy *l, CTy *r)
{
	/* TODO */
	return 0;
}

int 
sametype(CTy *l, CTy *r)
{
	/* TODO */
	switch(l->t) {
	case CVOID:
		if(r->t != CVOID)
			return 0;
		return 1;
	case CPRIM:
		if(r->t != CPRIM)
			return 0;
		if(l->Prim.issigned != r->Prim.issigned)
			return 0;
		if(l->Prim.type != r->Prim.type)
			return 0;
		return 1;
	}
	return 0;
}

int
isftype(CTy *t)
{
	if(t->t != CPRIM)
		return 0;
	switch(t->Prim.type){
	case PRIMFLOAT:
	case PRIMDOUBLE:
	case PRIMLDOUBLE:
		return 1;
	}
	return 0;
}

int
isitype(CTy *t)
{
	if(t->t != CPRIM)
		return 0;
	switch(t->Prim.type){
	case PRIMCHAR:
	case PRIMSHORT:
	case PRIMINT:
	case PRIMLONG:
	case PRIMLLONG:
		return 1;
	}
	return 0;
}

int
isarithtype(CTy *t)
{
	return isftype(t) || isitype(t);
}

int
isptr(CTy *t)
{
	return t->t == CPTR;
}

int
isfunc(CTy *t)
{
	return t->t == CFUNC;
}

int
isfuncptr(CTy *t)
{
	return isptr(t) && isfunc(t->Ptr.subty);
}

int
isstruct(CTy *t)
{
	return t->t == CSTRUCT;
}

int
isarray(CTy *t)
{
	return t->t == CARR;
}

StructMember *
getstructmember(CTy *t, char *n)
{
	int     i;
	StructMember *sm;
	
	if(isptr(t))
		t = t->Ptr.subty;
	if(!isstruct(t))
		panic("internal error");
	for(i = 0; i < t->Struct.members->len; i++) {
		sm = vecget(t->Struct.members, i);
		if(strcmp(n, sm->name) == 0)
			return sm;
	}
	return 0;
}

void
addstructmember(SrcPos *pos,CTy *t, char *name, CTy *membt)
{
	StructMember *sm,*subsm;
	int align, sz, i;

	sm = gcmalloc(sizeof(StructMember));
	sm->name = name;
	sm->type = membt;
	if(!isstruct(t))
		panic("internal error");
	if(t->Struct.isunion)
		panic("unimplemented addstructmember");
	if(sm->name == 0 && isstruct(sm->type)) {
		for(i = 0; i < sm->type->Struct.members->len; i++) {
			subsm = vecget(sm->type->Struct.members, i);
			addstructmember(pos, t, subsm->name, subsm->type);
		}
		return;
	}
	for(i = 0; i < t->Struct.members->len; i++) {
		subsm = vecget(t->Struct.members, i);
		if(strcmp(sm->name, subsm->name) == 0)
			errorposf(pos ,"struct already has a member named %s", sm->name);
	}
	sz = t->size;
	align = sm->type->align;
	if(sz % align)
		sz = sz + align - (sz % align);
	sm->offset = sz;
	sz += sm->type->size;
	t->size = sz;
	vecappend(t->Struct.members, sm);
}

CTy *
structmemberty(CTy *t, char *n)
{
	StructMember *sm;

	sm = getstructmember(t, n);
	if(!sm)
		return 0;
	return sm->type;
}

int
isassignable(CTy *to, CTy *from)
{
	if((isarithtype(to) || isptr(to)) &&
		(isarithtype(from) || isptr(from)))
		return 1;
	if(compatiblestruct(to, from))
		return 1;
	return 0;
}

uint64
getmaxval(CTy *l)
{
	switch(l->Prim.type) {
	case PRIMCHAR:
		if(l->Prim.issigned)
			return 0x7f;
		else
			return 0xff;
	case PRIMSHORT:
		if(l->Prim.issigned)
			return 0x7fff;
		else
			return  0xffff;
	case PRIMINT:
	case PRIMLONG:
		if(l->Prim.issigned)
			return 0x7fffffff;
		else
			return 0xffffffff;
	case PRIMLLONG:
		if(l->Prim.issigned)
			return 0x7fffffffffffffff;
		else
			return 0xffffffffffffffff;
	}
	panic("internal error");
	return 0;
}

int64
getminval(CTy *l)
{
	if(!l->Prim.issigned)
		return 0;
	switch(l->Prim.type) {
	case PRIMCHAR:
		return 0xff;
	case PRIMSHORT:
		return 0xffff;
	case PRIMINT:
	case PRIMLONG:
		return 0xffffffffl;
	case PRIMLLONG:
		return 0xffffffffffffffff;
	}
	panic("internal error");
	return 0;
}

int
canrepresent(CTy *l, CTy *r)
{
	if(!isitype(l) || !isitype(r))
		panic("internal error");
	return getmaxval(l) <= getmaxval(r) && getminval(l) >= getminval(r);
}

