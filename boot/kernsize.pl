#!/usr/bin/perl
my $sectors;
my $size;
$size = (stat($ARGV[0]))[7];
$test1 = 0xbbaaffcc;
$sectors = $size%512?$size/512+1:$size/512;
my $setup;
open(SETUP, "+<$ARGV[1]");
sysseek(SETUP, 0x34, 0);
$test1 = pack("L", $sectors);
syswrite(SETUP, $test1);

#$setup .= pack("L", $sectors);
#open(SETUP, ">$ARGV[1]");
#binmode SETUP;
#print SETUP $setup;

close SETUP;

