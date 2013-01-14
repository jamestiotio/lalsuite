#!/usr/bin/perl

use strict;
use warnings;

system("mkdir out");
die "mkdir failed: $?" if $?;
system("mkdir err");
die "mkdir failed: $?" if $?;

open(DAGFILE,">/home/egoetz/TwoSpect/ULfactor/dag") or die "Cannot write to /home/egoetz/TwoSpect/ULfactor/dag $!";
for(my $ii=0; $ii<100; $ii++) {
   print DAGFILE<<EOF;
JOB A$ii /home/egoetz/TwoSpect/ULfactor/condor
VARS A$ii JOBNUM="$ii"
EOF
   
   system("mkdir $ii");
   die "mkdir failed: $?" if $?;
}
close(DAGFILE);

open(CONDORFILE,">/home/egoetz/TwoSpect/ULfactor/condor") or die "Cannot write to /home/egoetz/TwoSpect/ULfactor/condor $!";
print CONDORFILE<<EOF;
universe=vanilla
executable=/home/egoetz/TwoSpect/ULfactor/run_MFD_TwoSpect_ULfactor.perl
input=/dev/null
output=/home/egoetz/TwoSpect/ULfactor/out/out.\$(JOBNUM)
error=/home/egoetz/TwoSpect/ULfactor/err/err.\$(JOBNUM)
arguments=\$(JOBNUM)
log=/local/user/egoetz/ULfactor.log
request_memory=2500
notification=Never
queue
EOF
close(CONDORFILE);
