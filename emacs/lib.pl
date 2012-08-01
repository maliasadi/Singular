#!/usr/local/bin/perl
###################################################################
#
# FILE:    lib.pl
# PURPOSE: generate template for library menu
# AUTHOR: obachman
####
$Usage = <<EOT;
$0 [-singular_dir singular_dir] libraries
Generates template for library menu
EOT
  
#
# default settings of command-line arguments
#
# here I expect libparse and LIB/*.lib
$Singular_Dir = "../Singular/";
#
# parse command line options
#
while (@ARGV && $ARGV[0] =~ /^-/)
{
  $_ = shift(@ARGV);
  if (/^-s(ingular_dir)?$/)  { $Singular_Dir = shift(@ARGV); next;}
  die "Unrecognized option: $_\n$Usage";
}
$libparse = "$Singular_Dir/libparse";
die "Can not find $libparse" unless -x $libparse;
$lib_dir = "$Singular_Dir/LIB";
die "$Singular_Dir/LIB is not a directory" unless -d $lib_dir;

for $lib (@ARGV)
{
  my $category;
  die "Can not read $lib_dir/$lib" unless -r "$lib_dir/$lib";
  open(CAT_PIPE, "$libparse -c -f $lib_dir/$lib|") || die "'$libparse -c -f $lib_dir/$lib' failed";
  
  while (<CAT_PIPE>)
  {
    $category = $_;
    last;
  }
  close(CAT_PIPE);
  $category = "Miscellaneous" unless $category;
  $libraries{$category} .= ($libraries{$category} ? " $lib" : $lib);
}

print <<EOT;
(setq singular-standard-libraries-with-categories
  '(
EOT
# font-lock-trick ' 
 
for $cat (sort {($b cmp $a)} keys %libraries)
{
  
  print <<EOT;
     ("$cat"
EOT
      for $lib (sort {($b cmp $a)} split (/\s+/, $libraries{$cat}))
      {
	print <<EOT;
       ("$lib")
EOT
      }
  print <<EOT 
     )
EOT
}

print "))\n";

  
__END__

[1  <text/plain; us-ascii (7bit)>]
Hallo Olaf!

Probier' es mal hiermit...

Das File lib-cmpl.el sollte jetzt folgendes Aussehen haben:

----[ snip ]----
; Do not edit this file: It was automatically generated by cmpl.pl
(setq singular-standard-libraries-with-categories
  '(
    ("Category_z" 
     ("z_file_z")
     ("z_file_y")
     ...
     ("z_file_a"))
    ("Category_y"
     ("y_file_z")
     ("y_file_y")
     ...
     ("y_file_a"))
    ...
    ("Category_a"
     ("a_file_z")
     ("a_file_y")
     ...
     ("a_file_a"))
    ("file_without_cat_z")
    ("file_without_cat_y")
    ...
    ("file_without_cat_a")
   )
----[ snip ]----

(beachte, dass sich der Name der lisp-Variablen geaendert hat!).  Wie
bisher stehen die Filenamen in umgekehrter alphabetischer Reihenfolge,
sowohl auf top-level, als auch in den sub-menues.  Das Menue wird im
Emacs genau so aufgebaut, wie angeben, nur in umgekehrter Reihenfolge.
Eintraege mit Categories und ohne koennen beliebig gemischt werden:

----[ snip ]----
(setq singular-standard-libraries-with-categories
  '(
    ("Category_z" 
     ("z_file_z")
     ("z_file_y")
     ("z_file_a"))
    ("file_without_cat_z")
    ("file_without_cat_y")
    ("Category_y"
     ("y_file_z")
     ("y_file_y")
     ("y_file_a"))
    ("file_without_cat_a")
   )
----[ snip ]----

Und, um dem ganzen noch eins draufzusetzen: man kann es sogar beliebig
tief schachteln!

----[ snip ]----
(setq singular-standard-libraries-with-categories
  '(
    ("Category_z" 
     ("z_file_z")
     ("Sub_Category_zz" ("zz_file1") ("zz_file2"))
     ("z_file_y")
     ("Sub_Category_aa" ("aa_file_a") ("aa_file_b")))
    ("file_without_cat_z")
    ("file_without_cat_y")
    ("Category_y"
     ("y_file_z")
     ("y_file_y")
     ("y_file_a"))
    ("file_without_cat_a")
   )
----[ snip ]----

Ich habe es auf emacs 20.4, xemacs 20.4 und xemacs 21.1-p3 getestet.
Ich hoffe, dass alles funktioniert!

Es sollte auch noch mit alten lib-cmpl.el-Datein funktionieren (obwohl 
sich der Variablen-Name geaendert hat)

Kannst Dich ja mal melden, ob alles wie gewollt klappt.

Viele Gruesse,
  T.


  
