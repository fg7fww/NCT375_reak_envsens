package require Tk

# Find the COM port used by the JLink adapter
# Procedure getSerialInterfaces copied from: http://wiki.tcl.tk/1838:

package require registry

proc getSerialInterfaces {} {
	set coms {}
	set res  {}
	set key "HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM"
	if {[catch {
		# The keys only available if the driver is loaded.
		foreach name [registry values $key] {
			set value [registry get $key $name]
			lappend coms $value
		}
	} fault]} {
		# return a empty list in case no serial interface is found
		return $res
	}

	set key HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Enum
	foreach el [registry keys $key] {
		foreach el1 [registry keys $key\\$el] {
			catch {
				foreach el2 [registry keys $key\\$el\\$el1] {
					set port [registry get "$key\\$el\\$el1\\$el2\\Device Parameters" PortName]
					if {[lsearch $coms $port] < 0} {
						continue
					}
					set friendlyName [registry get "$key\\$el\\$el1\\$el2" FriendlyName]
					lappend res [list $port $friendlyName]
				}
			}
		}
	}
	return [lsort $res]
}

set Com ""
foreach ComDef [getSerialInterfaces] {
	if {[regexp {JLink.*UART.*} [lindex $ComDef 1]]} {
		set Com [lindex $ComDef 0]
	}
}

if {$Com==""} {
	tk_messageBox -message "Cannot find JLink COM port" -type ok -icon error
	exit 1
}

pack [button .bClose -text Close -command {close $f; exit}]
pack [label .l -font "Helvetica 60 bold"]

proc GetData {chan} {
	if {[gets $chan line]>=0} {
		#puts $line
		if {[regexp {^(.*)C\s+(.*)%} $line {} Temp Hum]} {
			.l config -text "${Temp}°C\n${Hum}%"
		}
	}
}

set f [open $Com]
fconfigure $f -mode 115200,n,8,1
fileevent $f readable [list GetData $f]

