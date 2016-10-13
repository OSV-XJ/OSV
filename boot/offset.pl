#!/usr/bin/perl

my $size;
$size = (stat($ARGV[0]))[7];
printf "%d\n", $size%512?$size/512+2:$size/512+1;
