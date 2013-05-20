#!/usr/bin/perl 
#use strict;
#use warnings;
#use 5.010;


open (DATA, 'settings.csv') or die("Cannot open settings.csv");

print "static setting_t all_Settings[] = \{\n";

my $offset = 0;

while(<DATA>) {
    chomp;
# name, suffix, id, set_size, get_size, is_visible
    if (/^([\"]+.*[\"]+),(.*),([0-9]+[0-9]*),([0-9]+[0-9]*),([0-9]+[0-9]*),([0-1]+)/) {
	if($6 == 1) {
		$visible = "true";
	} else {
		$visible = "false";
	}
		print "\t\{$1,\"\",$3,$offset,$4,$visible, 0, false\},\n";
	$offset = $offset + $5;
    } else {if(/^(.*),(.*),([0-9]+[0-9]*),([0-9]+[0-9]*),([0-9]+[0-9]*),([0-1]+)/) {
	if($6 == "1") {
		$visible = "true";
	} else {
		$visible = "false";
	}
		print "\t\{\"$1\"\,\"\"\,$3,$offset,$4, $visible, 0, false\},\n";
	$offset = $offset + $5;
    }}

	#else{
	#if (/^([0-9]*),(.*),(.+.*),(.*),(.*),(.*)/) {
	#	print "\t\{$1,0\.0,$6,1\},\n"
	#}
	#}
}

print "};\n"
