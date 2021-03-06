#!/usr/bin/env perl
use strict;

my $pos;
my $sha1;
my @sha1line;
my $date;
my $descript;

# get the most recent git log (only 1)
my $gitlog=`git log -1`;
my $gitdescribe=`git describe --always`;
my @linelist=split('\n', $gitlog);
my $sha=`git rev-parse HEAD`;

$descript="Desc: $gitdescribe";
$sha1 = "SHA1: $sha";

# file for CD BUIld Ref ID base on GIT SHA1
open(OUTFILE, ">../../vnmr/adm/sha1/Build_Id.txt");

foreach my $elem (@linelist)
{
   # print "$elem, \n";
   # if Date line then grab it
   $pos = index($elem,"Date:");
   # print "$pos, \n";
   if ($pos != -1) 
   {
      $date = "$elem \n";
      # $pos = index($elem,":");
      # $date = substr($elem,$pos+3);
      # $date = "$elem\n";
      # print "$date\n";
   }
#   print "$list,\n";
#   print "CD SHA1 ID: \'$elem\',\n";
}

# write the two extracted/modified lines into the file
print OUTFILE "$descript";
print OUTFILE "$sha1";
print OUTFILE "$date";

close (OUTFILE);
