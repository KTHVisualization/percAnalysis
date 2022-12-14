
#~ Standard proc to see whether a certain step has already been processed.
#~ If you need something better, you may override it in your jobinfo
proc IsFileRemaining {FileNamePart InputFileName OutputFileName} {
    #~ Whether output exists or not
    if {[file exists $OutputFileName]} {
        #~ puts "$OutputFileName exists"
        return 0
    }
    
    #~ puts "$OutputFileName does not exists"
    return 1
}

proc WhichInviwo {InviwoExecutable} {
    if {$InviwoExecutable != ""} {return $InviwoExecutable}

    #~ Get the system we are running on
    set RunningMachine [exec uname -n]

    if {$RunningMachine == "KTH-6181"} {
        return "D:/Projekte/Inviwo/Output/bin/Release/inviwo.exe"
    } else {
        return [file dirname [info script]]/inviwo.exe
    }
}

proc FixFileName {FileName} {
    global NoCygwinPaths
    if {$NoCygwinPaths} {
        return [ exec cygpath -m $FileName ]
    } else {
        return $FileName
    }
}

proc FixFileNameList {FileNameList} {
    set FixedFileNameList [list]
    foreach FileName $FileNameList {
        lappend FixedFileNameList [FixFileName $FileName]
    }
    return $FixedFileNameList
}


#~ Standard jobname if no name is given
set JobName "jobinfo"

#~ The user does not really need to define a template for an output file in the jobinfo.
#~ Only needed if standard implementation of "IsFileRemaining" shall be used
set OutFileNameTemplate {Result_$FileNamePart}

set TestMode 0
set NoInviwoMode 0
set KeepScriptFiles 0
set NumThreads 1
set EveryN 1
set RemainingFilesOnly 0
set verbose 0
set NoCygwinPaths 1
set QSubMode 0
set ScriptFolder "."
set InviwoExecutable ""

if {[info exists argc]} {
   
    while {$argc} {
        set o [lindex $argv 0];
        if {$o == "--job"} {
            set JobName [lindex $argv 1];
            set rem 2;
        } elseif {$o == "--threads"} {
            set NumThreads [lindex $argv 1];
            set rem 2;
        } elseif {$o == "--start"} {
            set StartOffset [lindex $argv 1];
            set rem 2;
        } elseif {$o == "--end"} {
            set EndOffset [lindex $argv 1];
            set rem 2;
        } elseif {$o == "--every"} {
            set EveryN [lindex $argv 1];
            set rem 2;
        } elseif {$o == "--numfiles"} {
            set MaxNumberOfFiles [lindex $argv 1];
            set rem 2;
        } elseif {$o == "--remaining"} {
            set RemainingFilesOnly 1;
            set rem 1;
        } elseif {$o == "--test"} {
            set TestMode 1;
            #~ set verbose 1;
            set rem 1;
        } elseif {$o == "--noinviwo"} {
            set NoInviwoMode 1;
            set rem 1;
        } elseif {$o == "--keepscriptfiles"} {
            set KeepScriptFiles 1;
            set rem 1;
        } elseif {$o == "--verbose"} {
            set verbose 1;
            set rem 1;
        } elseif {$o == "--nocygwinpaths"} {
            set NoCygwinPaths 1;
            set rem 1;
        } elseif {$o == "--cygwinpaths"} {
            set NoCygwinPaths 0;
            set rem 1;
        } elseif {$o == "--qsub"} {
            set QSubMode 1
            set rem 1;
        } elseif {$o == "--scriptfolder"} {
            set ScriptFolder [lindex $argv 1];
            set rem 2;
        } elseif {$o == "--inviwoexecutable"} {
            set InviwoExecutable [lindex $argv 1];
            set rem 2;
        } elseif {$o == ""} {
            #~ Ignore an empty param, which might be generated by the thread calling
            set rem 1;
        } else {
            puts "Error: unknown option $o";
            exit
        }
        incr argc -$rem;
        set argv [lreplace $argv 0 [expr $rem - 1]];
    }
    if {[llength $argv] != 0} {
        puts "Wrong params"
    }
    
} else {
    puts "Start it with params"
    exit
}


#~ Set certain variables in qsub mode
if {$QSubMode} {
    set KeepScriptFiles 1
    set NoInviwoMode 1
    set NumThreads 1
}


#~ Read JobInfo
if {[catch {source $JobName}]} {
    puts "!\tError while sourcing '$JobName'"
    exit
}

#~ Read Script Template
set ScriptTemplate ""
puts "Reading script file '$ScriptName'"
if {[catch {set fp [open $ScriptName]}]} {
    puts "!\tError: could not read file '$ScriptName'";
    exit
} else {
    set ScriptTemplate [read $fp];
    close $fp;
}


#~ Get the input files and sort them in order to always have the same order of execution
set FileNameList [eval [subst {glob $InFilesGlobPattern} ] ]
set SortedFileNameList [lsort $FileNameList]

#~ Remove unwanted parts of the list
set ExecList [list]
set FoundStartOffset 0
set FoundEndOffset 0
set UniqueTestList [list]
set EveryCounter $EveryN
foreach InputFileName $SortedFileNameList {

    if {$FoundEndOffset == 1} {break}

    #~ Use only every nth input file
    if {$EveryCounter != $EveryN} {incr EveryCounter; continue;}
    set EveryCounter 1

    set FileNamePart [lindex [regexp -inline $UniqueFileNamePartRegexp $InputFileName] 1]

    #~ Check uniqueness - this is quite important if we want to run in parallel
    if {[lsearch $UniqueTestList $FileNamePart] >= 0} {
        #~ Not unique
        puts "The specified unique pattern is actually not unique."
        puts "You specified 'set UniqueFileNamePartRegexp {$UniqueFileNamePartRegexp}'."
        puts "==> The value '$FileNamePart' was found twice."
        puts "The simplest way to achieve uniqueness is to use the filename, i.e.:"
        puts "\tset UniqueFileNamePartRegexp {(.*)}"
        exit
    } else {
        #~ Unique - up to here
        lappend UniqueTestList $FileNamePart
    }

    if {[info exists StartOffset] && $FoundStartOffset == 0} {
        #~ Search for the offset
        if {$FileNamePart != $StartOffset} {
            #~ puts "Skipping $InputFileName"
            continue
        }
    }
    if {$FoundStartOffset == 0} {
        if {$verbose} {puts "$InputFileName will be the first data set to be used"}
        set FoundStartOffset 1
    }

    if {[info exists EndOffset]} {
        #~ Search for the offset
        if {$FileNamePart == $EndOffset} {
            if {$verbose} {puts "$InputFileName will be the last data set to be used"}
            set FoundEndOffset 1
        }
    }

    #~ Input Directory - may be used in the output template
    set InputDirectory [file dirname [file normalize $InputFileName]]
    
    #~ Get time value, if wanted - may be used in the output template
    set TimeValue  0
    if {[info exists TimeValueRegexp]} {
        set TimeValue [lindex [regexp -inline $TimeValueRegexp $InputFileName] 1]
    }

    #~ Name of the output(s)
    set OutputFileNames [list]
    if {[info exists OutFileNameTemplateList]} {
        foreach OutNameTemplate $OutFileNameTemplateList {
            lappend OutputFileNames [file normalize [subst $OutNameTemplate]]
        }
    } else {
        lappend OutputFileNames [file normalize [subst $OutFileNameTemplate]]
    }

    #~ Run only on files which have not been processed before (at least no output exists)?
    if {$RemainingFilesOnly} {
        set DoRunOnThisInput 0
        foreach SingleOutputFileName $OutputFileNames {
            if {[IsFileRemaining $FileNamePart $InputFileName $SingleOutputFileName]} {set DoRunOnThisInput 1}
        }
        if {!$DoRunOnThisInput} {continue}
    }

    #~ We use this data set; record its information
    lappend ExecList [list $FileNamePart [file normalize $InputFileName] $OutputFileNames $TimeValue]
    
    if {[info exists MaxNumberOfFiles]} {
        if {$MaxNumberOfFiles > 0 && [llength $ExecList] == $MaxNumberOfFiles} {break}
    }
}


#~ Something to do?
set NumExecFiles [llength $ExecList]
if {$NumExecFiles == 0} {
    puts "No input files to run on"
    exit
}
puts "Running on $NumExecFiles input files"


#~ Threads - run this job in parallel
#~ For this, we split up the input files in several chunks and execute ourselves

#~ If we have more threads than exetution files, we reduce the number of threads accordingly.
if { $NumThreads > $NumExecFiles} {set NumThreads $NumExecFiles}
 
if {$NumThreads > 1} {
    #~ Split up the input
    set NumFilesPerThread [expr $NumExecFiles / $NumThreads]
    #~ If there is a remainder of files we increment the number of files per thread
    if {$NumFilesPerThread * $NumThreads < $NumExecFiles} { incr NumFilesPerThread  }
    #~ Calculate how many threads do one execution file more ("full threads") and how many do one less.
    set NumOfThreadsHavingLess [expr $NumFilesPerThread * $NumThreads - $NumExecFiles]
    set NumOfFullThreads [expr $NumThreads - $NumOfThreadsHavingLess]
    if {$verbose} {puts "Number of files per thread $NumFilesPerThread for the first $NumOfFullThreads threads."}

    #~ Create Start/End Info for the threads
    set StartEndInfo [list]
    set StartIdx 0
    for {set i 0} {$i < $NumThreads} {incr i} {
        #~ The last threads do one file less, so decrement the number of files per thread when number of "full threads" is reached.
        if {$i == $NumOfFullThreads} {set NumFilesPerThread [expr $NumFilesPerThread - 1]}
        set EndIdx [expr $StartIdx + $NumFilesPerThread - 1]
        if {$i == [expr $NumThreads - 1]} {set EndIdx [expr $NumExecFiles - 1]}
        
        #~ Set Info
        set StartPart [lindex [lindex $ExecList $StartIdx] 0]
        set EndPart [lindex [lindex $ExecList $EndIdx] 0]
        lappend StartEndInfo $StartPart $EndPart
        puts "Thread $i runs on [expr $EndIdx - $StartIdx + 1] files \[$StartPart : $EndPart\]"
        
        #~ Get new StartIdx
        set StartIdx [expr $EndIdx + 1]
    }

    #~ Start the threads (always non-verbose)
    if {$RemainingFilesOnly} {set ParamRemainingFilesOnly "--remaining"} else {set ParamRemainingFilesOnly ""}
    if {$TestMode} {set ParamTestMode "--test"} else {set ParamTestMode ""}
    if {$NoInviwoMode} {set ParamNoInviwoMode "--noinviwo"} else {set ParamNoInviwoMode ""}
    if {$KeepScriptFiles} {set ParamKeepScriptFiles "--keepscriptfiles"} else {set ParamKeepScriptFiles ""}
    if {$InviwoExecutable != ""} {
        set ParamInviwoExeParam "--inviwoexecutable"
        set ParamInviwoExecutable $InviwoExecutable
    } else {
        set ParamInviwoExeParam ""
        set ParamInviwoExecutable ""
    }

    foreach {StartPart EndPart} $StartEndInfo {
        #~ puts "[info nameofexecutable] [info script] --job $JobName --start $StartPart --end $EndPart --every $EveryN $ParamRemainingFilesOnly $ParamTestMode $ParamNoInviwoMode $ParamKeepScriptFiles $ParamInviwoExeParam $ParamInviwoExecutable &"
        exec [info nameofexecutable] [info script] --job $JobName --start $StartPart --end $EndPart --every $EveryN $ParamRemainingFilesOnly $ParamTestMode $ParamNoInviwoMode $ParamKeepScriptFiles $ParamInviwoExeParam $ParamInviwoExecutable &
        #~ We wait so that the resources are used out of sync
        exec sleep 10
    }

    #~ This instance does not run inviwo
    exit
}


#~ Name of inviwo
set InviwoExecutable [WhichInviwo $InviwoExecutable]

#~ Filename of qsub shell script and folder for all qsub stuff
set QSubShellName [file join $ScriptFolder "${ScriptName}_QSub.sh"]
set QSubDirAbsolute [file normalize $ScriptFolder]


if {!$TestMode} {
    #~ Create the working folder for the scripts and so on
    if {$ScriptFolder != "" && ![file exists $ScriptFolder]} {
        if {$verbose} {puts "Creating directory $ScriptFolder"}
        file mkdir $ScriptFolder
    }

    #~ Add qsub stuff
    if {$QSubMode} {
        if {[catch {set QSubShellFile [open $QSubShellName "w"]}]} {
            puts "!\tError: could not open file '$QSubShellName' for writing (Permissions?)";
            exit
        } else {
            puts $QSubShellFile "#!/bin/csh"
            puts $QSubShellFile "setenv OMP_NUM_THREADS 1"

            puts $QSubShellFile "echo Executing task number \$SGE_TASK_ID"
            puts $QSubShellFile "vncserverrun $InviwoExecutable --nosplash --logfile [file join $QSubDirAbsolute ${ScriptName}_QSub_\$SGE_TASK_ID.log] --pythonScript [file join $QSubDirAbsolute ${ScriptName}_QSub_\$SGE_TASK_ID.py --quit]"
            close $QSubShellFile
        }
    }
}


#~ Run the job
set FoundStartOffset 0
set FoundEndOffset 0
set ItemIndex 0
foreach ExecItem $ExecList {

    incr ItemIndex

    set FileNamePart [lindex $ExecItem 0]
    set InputFileName [FixFileName [lindex $ExecItem 1]]
    set InputDirectory [FixFileName [file dirname $InputFileName]]
    set OutputFileNames [FixFileNameList [lindex $ExecItem 2]]
    set TimeValue [lindex $ExecItem 3]

    if {$verbose} {
        puts "($FileNamePart) Running on $InputFileName"
        foreach SingleOutputFileName $OutputFileNames {
            puts "\t==> Creating $SingleOutputFileName"
        }
    }

    if {!$TestMode} {

        #~ Write py script
        if {$QSubMode} {
            set NewScriptFileName [file join $ScriptFolder "${ScriptName}_QSub_$ItemIndex.py"]
        } else {
            set SafeFileNamePart [regsub -all {/|:|\\|\s} $FileNamePart ""]
            set NewScriptFileName [file join $ScriptFolder "$ScriptName$SafeFileNamePart.py"]
        }
        if {[catch {set NewScriptFile [open $NewScriptFileName "w"]}]} {
            puts "!\tError: could not open file '$NewScriptFileName' for writing (Permissions?)";
            exit
        } else {
            puts $NewScriptFile "# RunForAll.tcl Variables -------------"
            puts $NewScriptFile "InputFileName = \"$InputFileName\""
            puts $NewScriptFile "InputDirectory = \"$InputDirectory\""
            puts $NewScriptFile "FileNamePart = \"$FileNamePart\""
            set OutputFileNamesPyList ""
            foreach SingleOutputFileName $OutputFileNames {
                append OutputFileNamesPyList \"$SingleOutputFileName\"\,
            }
            puts $NewScriptFile "OutputFileNames = \[$OutputFileNamesPyList\]"
            puts $NewScriptFile "OutputFileName = \"[lindex $OutputFileNames end]\""
            puts $NewScriptFile "TimeValue = \"$TimeValue\""
            puts $NewScriptFile "# RunForAll.tcl Variables -------------"
            puts $NewScriptFile ""
            puts $NewScriptFile $ScriptTemplate
            puts $NewScriptFile ""
            close $NewScriptFile
        }
        
        #~ Ensure that the output directory exists
        foreach SingleOutputFileName $OutputFileNames {
            set OutputDirectory [file dirname $SingleOutputFileName]
            if {![file exists $OutputDirectory]} {
                if {$verbose} {puts "Creating directory $OutputDirectory"}
                file mkdir $OutputDirectory
            }
        }

        if {!$NoInviwoMode} {
            #~ Start Inviwo with the new script
            catch {exec nice -n 10 $InviwoExecutable --nosplash --pythonScript $NewScriptFileName --quit}
        }

        if {!$KeepScriptFiles} {
            #~ Remove new script
            file delete $NewScriptFileName
        }
    }
}

if {$QSubMode} {
    puts "Start the qsub job like this:\n"
    puts "\tqsub -q all.q -notify -e \"$QSubDirAbsolute\" -o \"$QSubDirAbsolute\" -t 1-$NumExecFiles $QSubShellName"
    puts "\nYou can monitor the progress using qstat. You can safely disconnect after submission."
}


