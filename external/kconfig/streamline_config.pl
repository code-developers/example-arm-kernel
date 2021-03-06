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

# Get the build source and top level Kconfig file (passed in)
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

sub read_kconfig {
    my ($kconfig) = @_;

    my $state = "NONE";
    my $config;
    my @kconfigs;

    my $cont = 0;
    my $line;

    my $source = "$ksource/$kconfig";
    my $last_source = "";

    # Check for any environment variables used
    while ($source =~ /\$(\w+)/ && $last_source ne $source) {
	my $env = $1;
	$last_source = $source;
	$source =~ s/\$$env/$ENV{$env}/;
    }

    open(KIN, "$source") || die "Can't open $kconfig";
    while (<KIN>) {
	chomp;

	# Make sure that lines ending with \ continue
	if ($cont) {
	    $_ = $line . " " . $_;
	}

	if (s/\\$//) {
	    $cont = 1;
	    $line = $_;
	    next;
	}

	$cont = 0;

	# collect any Kconfig sources
	if (/^source\s*"(.*)"/) {
	    $kconfigs[$#kconfigs+1] = $1;
	}

	# configs found
	if (/^\s*(menu)?config\s+(\S+)\s*$/) {
	    $state = "NEW";
	    $config = $2;

	    for (my $i = 0; $i < $iflevel; $i++) {
		if ($i) {
		    $depends{$config} .= " " . $ifdeps[$i];
		} else {
		    $depends{$config} = $ifdeps[$i];
		}
		$state = "DEP";
	    }

	# collect the depends for the config
	} elsif ($state eq "NEW" && /^\s*depends\s+on\s+(.*)$/) {
	    $state = "DEP";
	    $depends{$config} = $1;
	} elsif ($state eq "DEP" && /^\s*depends\s+on\s+(.*)$/) {
	    $depends{$config} .= " " . $1;

	# Get the configs that select this config
	} elsif ($state ne "NONE" && /^\s*select\s+(\S+)/) {
	    if (defined($selects{$1})) {
		$selects{$1} .= " " . $config;
	    } else {
		$selects{$1} = $config;
	    }

	# configs without prompts must be selected
	} elsif ($state ne "NONE" && /^\s*tristate\s\S/) {
	    # note if the config has a prompt
	    $prompts{$config} = 1;

	# Check for if statements
	} elsif (/^if\s+(.*\S)\s*$/) {
	    my $deps = $1;
	    # remove beginning and ending non text
	    $deps =~ s/^[^a-zA-Z0-9_]*//;
	    $deps =~ s/[^a-zA-Z0-9_]*$//;

	    my @deps = split /[^a-zA-Z0-9_]+/, $deps;

	    $ifdeps[$iflevel++] = join ':', @deps;

	} elsif (/^endif/) {

	    $iflevel-- if ($iflevel);

	# stop on "help"
	} elsif (/^\s*help\s*$/) {
	    $state = "NONE";
	}
    }
    close(KIN);

    # read in any configs that were found.
    foreach $kconfig (@kconfigs) {
	if (!defined($read_kconfigs{$kconfig})) {
	    $read_kconfigs{$kconfig} = 1;
	    read_kconfig($kconfig);
	}
    }
}

if ($kconfig) {
    read_kconfig($kconfig);
}

# Read all Makefiles to map the configs to the objects
foreach my $makefile (@makefiles) {

    my $cont = 0;

    open(MIN,$makefile) || die "Can't open $makefile";
    while (<MIN>) {
	my $objs;

	# is this a line after a line with a backslash?
	if ($cont && /(\S.*)$/) {
	    $objs = $1;
	}
	$cont = 0;

	# collect objects after obj-$(CONFIG_FOO_BAR)
	if (/obj-\$\((CONFIG_[^\)]*)\)\s*[+:]?=\s*(.*)/) {
	    $var = $1;
	    $objs = $2;
	}
	if (defined($objs)) {
	    # test if the line ends with a backslash
	    if ($objs =~ m,(.*)\\$,) {
		$objs = $1;
		$cont = 1;
	    }

	    foreach my $obj (split /\s+/,$objs) {
		$obj =~ s/-/_/g;
		if ($obj =~ /(.*)\.o$/) {
		    # Objects may be enabled by more than one config.
		    # Store configs in an array.
		    my @arr;

		    if (defined($objects{$1})) {
			@arr = @{$objects{$1}};
		    }

		    $arr[$#arr+1] = $var;

		    # The objects have a hash mapping to a reference
		    # of an array of configs.
		    $objects{$1} = \@arr;
		}
	    }
	}
    }
    close(MIN);
}

my %modules;

if (defined($lsmod_file)) {
    if ( ! -f $lsmod_file) {
	die "$lsmod_file not found";
    }
    if ( -x $lsmod_file) {
	# the file is executable, run it
	open(LIN, "$lsmod_file|");
    } else {
	# Just read the contents
	open(LIN, "$lsmod_file");
    }
} else {

    # see what modules are loaded on this system
    my $lsmod;

    foreach my $dir ( ("/sbin", "/bin", "/usr/sbin", "/usr/bin") ) {
	if ( -x "$dir/lsmod" ) {
	    $lsmod = "$dir/lsmod";
	    last;
	}
}
    if (!defined($lsmod)) {
	# try just the path
	$lsmod = "lsmod";
    }

    open(LIN,"$lsmod|") || die "Can not call lsmod with $lsmod";
}

while (<LIN>) {
	next if (/^Module/);  # Skip the first line.
	if (/^(\S+)/) {
		$modules{$1} = 1;
	}
}
close (LIN);

# add to the configs hash all configs that are needed to enable
# a loaded module.
my %configs;
foreach my $module (keys(%modules)) {
    if (defined($objects{$module})) {
	my @arr = @{$objects{$module}};
	foreach my $conf (@arr) {
	    $configs{$conf} = $module;
	}
    } else {
	# Most likely, someone has a custom (binary?) module loaded.
	print STDERR "$module config not found!!\n";
    }
}

my $valid = "A-Za-z_0-9";
my $repeat = 1;


sub parse_config_dep_select
{
    my ($p) = @_;

    while ($p =~ /[$valid]/) {

	if ($p =~ /^[^$valid]*([$valid]+)/) {
	    my $conf = "CONFIG_" . $1;

	    $p =~ s/^[^$valid]*[$valid]+//;

	    if (!defined($configs{$conf})) {
		# We must make sure that this config has its
		# dependencies met.
		$repeat = 1; # do again
		$configs{$conf} = 1;
	    }
	} else {
	    die "this should never happen";
	}
    }
}

while ($repeat) {
    $repeat = 0;

    foreach my $config (keys %configs) {
	$config =~ s/^CONFIG_//;

	if (defined($depends{$config})) {
	    # This config has dependencies. Make sure they are also included
	    parse_config_dep_select $depends{$config};
	}

	if (defined($prompts{$config}) || !defined($selects{$config})) {
	    next;
	}

	# config has no prompt and must be selected.
	parse_config_dep_select $selects{$config};
    }
}

my %setconfigs;

# Finally, read the .config file and turn off any module enabled that
# we could not find a reason to keep enabled.
while(<CIN>) {

    if (/CONFIG_IKCONFIG/) {
	if (/# CONFIG_IKCONFIG is not set/) {
	    # enable IKCONFIG at least as a module
	    print "CONFIG_IKCONFIG=m\n";
	    # don't ask about PROC
	    print "# CONFIG_IKCONFIG_PROC is not set\n";
	} else {
	    print;
	}
	next;
    }

    if (/^(CONFIG.*)=(m|y)/) {
	if (defined($configs{$1})) {
	    $setconfigs{$1} = $2;
	} elsif ($2 eq "m") {
	    print "# $1 is not set\n";
	    next;
	}
    }
    print;
}
close(CIN);

loop:
foreach my $module (keys(%modules)) {
    if (defined($objects{$module})) {
	my @arr = @{$objects{$module}};
	foreach my $conf (@arr) {
	    if (defined($setconfigs{$conf})) {
		next loop;
	    }
	}
	print STDERR "module $module did not have configs";
	foreach my $conf (@arr) {
	    print STDERR " " , $conf;
	}
	print STDERR "\n";
    }
}
