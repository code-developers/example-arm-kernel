#!/usr/bin/env perl

use strict;

my $config = ".config";

my $uname = `uname -r`;
chomp $uname;

my @searchconfigs = (
	{
	    "file" => ".config",
	    "exec" => "cat",
	},
	{
	    "file" => "/proc/config.gz",
	    "exec" => "zcat",
	},
	{
	    "file" => "/boot/config-$uname",
	    "exec" => "cat",
	},
	{
	    "file" => "/boot/vmlinuz-$uname",
	    "exec" => "scripts/extract-ikconfig",
	    "test" => "scripts/extract-ikconfig",
	},
	{
	    "file" => "vmlinux",
	    "exec" => "scripts/extract-ikconfig",
	    "test" => "scripts/extract-ikconfig",
	},
	{
	    "file" => "/lib/modules/$uname/kernel/kernel/configs.ko",
	    "exec" => "scripts/extract-ikconfig",
	    "test" => "scripts/extract-ikconfig",
	},
	{
	    "file" => "kernel/configs.ko",
	    "exec" => "scripts/extract-ikconfig",
	    "test" => "scripts/extract-ikconfig",
	},
	{
	    "file" => "kernel/configs.o",
	    "exec" => "scripts/extract-ikconfig",
	    "test" => "scripts/extract-ikconfig",
	},
);

sub find_config {
    foreach my $conf (@searchconfigs) {
	my $file = $conf->{"file"};

	next if ( ! -f "$file");

	if (defined($conf->{"test"})) {
	    `$conf->{"test"} $conf->{"file"} 2>/dev/null`;
	    next if ($?);
	}

	my $exec = $conf->{"exec"};

	print STDERR "using config: '$file'\n";

	open(CIN, "$exec $file |") || die "Failed to run $exec $file";
	return;
    }
    die "No config file found";
}

find_config;

my $ksource = $ARGV[0];
my $kconfig = $ARGV[1];
my $lsmod_file = $ARGV[2];

my @makefiles = `find $ksource -name Makefile 2>/dev/null`;
chomp @makefiles;

my %depends;
my %selects;
my %prompts;
my %objects;
my $var;
my $iflevel = 0;
my @ifdeps;

# prevent recursion
my %read_kconfigs;