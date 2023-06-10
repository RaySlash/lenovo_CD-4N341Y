#!/usr/bin/perl -W
# (c) 2017, Jianxin Pan.

my $top = ".";
my $err_cnt = 0;
my $k_v = 3;
my $exit = 0;
my $git_format_link="http://wiki-china.amlogic.com/Platform/Bootloader/Bootloader_commit_message_format";

# Get Kernel Version

sub get_kernel_version
{
	my $file = "$top/Makefile";
	#print "1. Get kernel version: ";
	open F, $file or die "Can't open file '$file' for read. $!" ;
	my $line = <F>;
	close F;
	if( $line =~ /^VERSION = (\w)$/)
	{
		$k_v = $1;
	}
	#print "$k_v\n";
}



#Check mesonxx_defconfig
sub check_defconfig
{
	my $err = 0;
	#print "2. Check meson_defconfig: ";
	`make ARCH=arm64 meson64_defconfig  1> /dev/null 2>&1`;
	`make ARCH=arm64 savedefconfig 1> /dev/null 2>&1`;
	`rm -fr .config`;
	my $diff = `diff defconfig ./arch/arm64/configs/meson64_defconfig`;
	if(length($diff))
	{
		$err_cnt += 1;
		$err = 1;
		$err_msg .= "    $err_cnt: meson64_defconfig not generated by savedefconfig\n";
	}

	if($k_v >=4)
	{
		`make ARCH=arm meson32_defconfig  1> /dev/null 2>&1`;
		`make ARCH=arm savedefconfig  1> /dev/null 2>&1`;
		`rm -fr .config`;
		$diff = `diff defconfig ./arch/arm/configs/meson32_defconfig`;
		if(length($diff))
		{
			$err_cnt += 1;
			$err = 1;
			$err_msg .= "    $err_cnt: meson32_defconfig not generated by savedefconfig\n";
		}
	}
	#print $err ? "fail\n" : "success\n";
}


my $MAX_LEN	=	80;

sub check_msg_common
{
	my $line = pop(@_);
	my $lnum = pop(@_);

	if( (length($line) > ($MAX_LEN + 4) ) && ($lnum > 4) )
	{	#Line over 80 characters is not allowed.
		$line =~ s/^(\s){4}//;
		$err_cnt += 1;
		$err_msg .= "    $err_cnt: Line over $MAX_LEN characters: <$line>\n";
	}

	if ( ($line =~ /\s+$/) && $line !~/^(\s){4}$/ )
	{ #No space at the end of line
		$err_cnt += 1;
		$line =~ s/^(\s){4}//;
		$err_msg .= "    $err_cnt: No space at the end of line: <$line>\n";
	}
}

sub check_msg_49
{
	my $line = pop(@_);
	my $lnum = pop(@_);

	if( $lnum == 5 )
	{
		if ($line !~ /^(\s){4}([\w]+:\s){1,2}[\w]+.*[\S]+$/)
		{
			$err_cnt += 1;
			$line =~ s/^(\s){4}//;
			$err_msg .= "    $err_cnt: <module: message>, but <$line>\n";
		}
		elsif ( $line =~ /PD\#/i )
		{
			$err_cnt += 1;
			$line =~ s/^(\s){4}//;
			$err_msg .= "    $err_cnt: No PD#XXXX in the first line for 4.9, but <$line>\n";
		}
		elsif ( $line =~ /(kernel)/i)
		{
			$err_cnt += 1;
			$line =~ s/^(\s){4}//;
			$err_msg .= "    $err_cnt: No 'kernel' in kernel commit message, but <$line>\n";
		}
	}

	if( $lnum == 6 )
	{
		if( $line !~/^(\s){4}$/ )
		{
			$err_cnt += 1;
			$line =~ s/^(\s){4}//;
			$err_msg .= "    $err_cnt: This line should be empty, but <$line>\n";
		}
	}

	if( $lnum == 7 )
	{
		if( $line !~ /^(\s){4}PD\#/ )
		{
			$err_cnt += 1;
			$line =~ s/^(\s){4}//;
			$err_msg .= "    $err_cnt: <PD#XXXX: detailed description>, but <$line>\n";
		}
	}
}

sub check_msg_49_2
{
	my $msg = `git cat-file commit HEAD~0 | sed '1,/\^\$/d'`;
	my @str = split /[\n][\n]/, $msg;
	my $i = 0;
	my $len = @str;

	if( $len < 4 )
	{
		$err_cnt += 5;
		$err_msg .= "	module: message [n/m]\n\n";
		$err_msg .= "	PD#SWPL-XXXX\n\n";
		$err_msg .= "	Problem:\n	detailed description\n\n";
		$err_msg .= "	Solution:\n	detailed description\n\n";
		$err_msg .= "	Verify:\n	detailed description\n\n";
		return -1;
	}

	if( $str[$i] !~ /^([\w]+:\s){1,2}.+(\s)\[[\d]\/[\d]\]$/ )
	{
		$err_cnt += 1;
		$err_msg .= "	$err_cnt: module: message\n";
	}
	elsif( $str[$i] =~ /(kernel)/i )
	{
		$err_cnt += 1;
		$err_msg .= "	$err_cnt: Should be no 'kernel' in kernel commit message\n";
	}

	if( $str[++ $i] !~ /^PD\#SWPL-.+(\d)$/ )
	{
		$err_cnt += 1;
		$err_msg .= "	$err_cnt: PD#SWPL-XXXX\n";

	}

	if( $str[++ $i] !~ /^Problem:[\n].+/ )
	{
		$err_cnt += 1;
		$err_msg .= "	$err_cnt: Problem:\n	detailed description\n";
	}

	$i += 1;
	while( $str[$i] !~ /^Solution:[\n].+/ && $str[$i] !~ /^Change-Id:/ && $str[$i] !~ /^Verify:[\n].+/ && ($i + 1) < $len)
	{
		$i = $i + 1;
	}

	if( $str[$i] !~ /^Solution:[\n].+/ )
	{
		$err_cnt += 1;
		$err_msg .= "	$err_cnt: Solution:\n	detailed description\n";
	}

	if( $str[$i] =~ /^Change-Id:/ )
	{
		$err_cnt += 1;
		$err_msg .= "	$err_cnt: Verify:\n	detailed description\n";
		return -1;
	}

	while( $str[$i] !~ /^Verify:[\n].+/ && $str[$i] !~ /^Change-Id:/ && ($i + 1) < $len )
	{
		$i += 1;
	}

	if( $str[$i] !~ /^Verify:[\n].+/ )
	{
		$err_cnt += 1;
		$err_msg .= "	$err_cnt: Verify:\n	detailed description\n";
	}
}

sub check_msg_314
{
	my $line = pop(@_);
	my $lnum = pop(@_);

	if( $lnum == 5 )
	{
		if ($line !~ /^(\s){4}PD\#.+:\s([\w]+:\s){1,2}[\w]+.*[\S]+$/)
		{
			$err_cnt += 1;
			$line =~ s/^(\s){4}//;
			$err_msg .= "    $err_cnt: <PD#XXXX/PD#SWPL-XXXX: module_id: commit message>, but <$line>\n";
		}
		elsif ( $line =~ /(kernel)/i)
		{
			$err_cnt += 1;
			$line =~ s/^(\s){4}//;
			$err_msg .= "    $err_cnt: No 'kerenl' in this line, but <$line>\n";
		}
	}

	if( $lnum == 6 )
	{
		if( $line !~/^(\s){4}$/ )
		{
			$err_cnt += 1;
			$line =~ s/^(\s){4}//;
			$err_msg .= "  $err_cnt: This line should be empty, but <$line>\n";
		}
	}
}


sub check_commit_msg
{

	my $lnum = 0;
	my $err = 0;
	my $result = 0;
	my $commit;
	my $FILE;

	open($FILE, '<&STDIN');

	while (<$FILE>) {
		chomp;
		my $line = $_;


		if( $line =~ /^commit\s([0-9a-z])+$/)
		{
			$lnum = 0;
			$commit = $line;
			$skip = 0;
		}
		$lnum ++;


		if( ($lnum ==2) && ($line =~ /^Merge: /))
		{
			#$skip =1;			#Don't Check branch merge
		}
		if( ($lnum==2) && ($line !~ /^Author: .*\@amlogic\.com\>$/))
		{
			#$skip =1;			#Don't Check commit which is not from amlogic
		}

		if( $err == 1)
		{
			$skip = 1;
			$err = 0;
			$result = 1;
		}
		if( $skip ==1)
		{
			next;
		}

		check_msg_common($lnum, $line);
		if ( $k_v < 4)
		{
			check_msg_314($lnum, $line);
		}
	}
	close $FILE;
	if ($k_v >= 4)
	{
		check_msg_49_2;
	}
}

sub out_review
{
	my $out_msg = "";
	my $out_file = "../output/review.txt";

	if ($err_cnt)
	{
		$out_msg = <<END;
\$ git log -1 | ./scripts/amlogic/merge_pre_check.pl -; total $err_cnt errors.
${err_msg}

END

		#open O, "> $out_file" or die "Can't Open $out_file For Write \n";
		#print O $out_msg;
		#close O;
		$exit = 1;
		print $out_msg;
		print "Please refer to:\n	$git_format_link\n";
	}
	else
	{
		print "";
	}
}

#start

my $err_msg_p = "\nCommit Pre check failed. Total $err_cnt errors.\n";

#Get Kernel Version

get_kernel_version();

#Check meson_defconfig
check_defconfig();

#Check commit message

check_commit_msg();

out_review();
#out
exit $exit;