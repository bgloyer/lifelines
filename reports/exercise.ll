/*
 * @progname       exercise
 * @version        0.8 (2001/04/21)
 * @author         Perry Rapp
 * @category       test
 * @output         mixed
 * @description    

Exercises lots of functions
cycling through most of the database
(this was adapted from my scrubLiving report)
and ending with a gengedcomstrong dump.

The exerciseStrings proc (although not complete)
actually tests the results & prints if a wrong
result is found.
*/

global(dead)
global(cutoff_yr)



proc main()
{
/*  getintmsg(cutoff_yr, "Enter number repeats") */

  "database: " database() nl()
  "version: " version() nl()

  indiset(iset)
  forindi (person, pnum) {
    if (isLivingPerson(person)) {
"living: " name(person) nl()
      call outputLivingIndi(person)
	  addtoset(iset,person,1)
    } else {
"dead" name(person) nl()
      call output(person)
	  addtoset(iset,person,0)
    }
    call exerciseIndi(person)
  }
  forfam (fam, fnum) {
    if (isLivingFam(fam)) {
      call outputLivingFam(fam)
    } else {
      call output(fam)
    }
  }
  forsour (sour,fnum) {
    call output(sour)
  }
  call exerciseStrings()

  gengedcomstrong(iset)
}

proc output(record)
{
  traverse (root(record), node, level) {
    if (and(ne(tag(node),"SOUR"),ne(tag(node),"NOTE"))) {
      d(level) " " xref(node) " " tag(node) " " value(node)
      nl()
    }
  }
}

proc outputLivingIndi(indi)
{
  "0 @" key(indi) "@ INDI" nl()
  "1 NAME " fullname(indi,0,1,50) nl()
  fornodes(inode(indi), node) {
    if (isFamilyPtr(node)) {
      "1 " xref(node) " " tag(node) " " value(node)
      nl()
    }
  }
}


proc outputLivingFam(fam)
{
  "0 @" key(fam) "@ FAM" nl()
  fornodes(root(fam), node) {
    if (isMemberPtr(node)) {
      "1 " xref(node) " " tag(node) " " value(node)
      nl()
    }
  }
}

func isLivingFam(fam)
{
  fornodes(root(fam), node) {
    if (isMemberPtr(node)) {
      if (isLivingPerson(indi(value(node)))) { return (1) }
    }
  }
  return (0)
}

func isLivingPerson(indi)
{
  if (death(indi)) { return (0) }
  if (birth(indi)) {
    extractdate(birth(indi),day,mon,yr)
    if (and(gt(yr,300),lt(yr,cutoff_yr))) { return (0) }
  }
  return (1)
}



func isFamilyPtr (node) {
  if (eq(tag(node),"FAMC")) { return (1) }
  if (eq(tag(node),"FAMS")) { return (1) }
  return (0)
}

func isMemberPtr (node) {
  if (eq(tag(node),"HUSB")) { return (1) }
  if (eq(tag(node),"WIFE")) { return (1) }
  if (eq(tag(node),"CHIL")) { return (1) }
  return (0)
}

proc exerciseIndi(indi)
{
  list(lst)
  set(em, empty(lst))
  enqueue(lst, indi)
  push(lst, father(indi))
  requeue(lst, mother(indi))
  set(junk,pop(lst))
  setel(lst, 1, nextsib(indi))
  forlist(lst, el, count)
  {
    name(el) " " d(count) nl()
  }
  table(tbl)
  insert(tbl, "bob", indi)
  set(thing, lookup(tbl, "bob"))
  indiset(iset)
  addtoset(iset,indi,"bob")
  set(iset,union(iset,parentset(iset)))
  addtoset(iset,indi,"jerry")
  addtoset(iset,father(indi), "dad")
  addtoset(iset,mother(indi), "mom")
  addtoset(iset,nextsib(indi), "bro")
  spouses(indi,spouse,fam,num)
  {
    addtoset(iset,spouse,fam)
	"spouse: " fullname(spouse, true, true, 20) nl()
  }
  families(indi,fam,spouse,num)
  {
    addtoset(iset,spouse,num)
	"family: " key(fam) nl()
  }
  addtoset(iset,nextindi(indi),"next")
  addtoset(iset,previndi(indi),"prev")
  set(p,99)
  "name: " name(indi) nl()
  "title: " title(indi) nl()
  "key: " key(indi) nl()
  parents(indi) nl()
  "fullname(12): " fullname(indi,true,true,12) nl()
  "surname: " surname(indi) nl()
  "givens: " givens(indi) nl()
  "trimname(8): " trimname(indi,8) nl()
  lock(indi)
  call dumpnode("birth", birth(indi))
  call dumpnodetr("death", death(indi))
  unlock(indi)
}

proc dumpnode(desc, node)
{
  if (node)
  {
    desc ": " xref(node) " " tag(node) " " value(node)
    fornodes(node, child)
    {
      call dumpnode2(child)
    }
  }
}

proc dumpnode2(node)
{
  xref(node) " " tag(node) " " value(node)
  fornodes(node, child)
  {
    call dumpnode2(child)
  }
}

proc dumpnodetr(desc, node)
{
  if (node)
  {
    desc ": " xref(node) " " tag(node) " " value(node) nl()
    traverse(node, child,lvl)
    {
      xref(node) " " tag(node) " " value(node) nl()
    }
  }
}

proc reportfail(str)
{
  print(str)
  str nl()
}

/* test some string functions against 
defined & undefined strings */
proc exerciseStrings()
{
  set(str,"hey")
  set(str2,upper(str))
  if (ne(str2,"HEY")) {
    call reportfail("upper FAILED")
  }
  set(str3,upper(undef))
  set(str4,capitalize(str))
  if (ne(str4,"Hey")) {
    call reportfail("capitalize FAILED")
  }
  set(str5,capitalize(undef))
  set(str6,concat(str2,str4))
  if (ne(str6,"HEYHey")) {
    call reportfail("concat FAILED")
  }
  set(str7,concat(str3,str5))
  if (ne(str7,undef)) {
    call reportfail("concat FAILED on undefs")
  }
  set(str8,lower(str4))
  if (ne(str8,"hey")) {
    call reportfail("lower FAILED")
  }
  set(str9,lower(undef))
  if (ne(str9,undef)) {
    call reportfail("lower FAILED on undef")
  }
  set(str10,alpha(3))
  if(ne(str10,"c")) {
    call reportfail("alpha(3) FAILED")
  }
  set(str10,roman(4))
  if(ne(str10,"iv")) {
    call reportfail("roman(4) FAILED")
  }
  set(str11,d(43))
  if(ne(str11,"43")) {
    call reportfail("d(43) FAILED")
  }
  set(str12,card(4))
  if(ne(str12,"four")) {
    call reportfail("card(4) FAILED")
  }
  set(str13,ord(5))
  if(ne(str13,"fifth")) {
    call reportfail("ord(5) FAILED")
  }
  if (ne(strcmp("alpha","beta"),-1)) {
    call reportfail("strcmp(alpha,beta) FAILED")
  }
  if (ne(strcmp("gamma","delta"),1)) {
    call reportfail("strcmp(gamma,delta) FAILED")
  }
  if (ne(strcmp("zeta","zeta"),0)) {
    call reportfail("strcmp(zeta,zeta) FAILED")
  }
  if (ne(strcmp(undef,""),0)) {
    call reportfail("strcmp(undef,) FAILED")
  }
}