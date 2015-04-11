; Create unique sections for specific section names
; Must append _rankname to the community to add your own sections

; Only one listed item from each list will be randomly selected
; section:addon_flag:ammo_type:ammo_min:ammo_max
; EXAMPLES
; wpn_l85:5:1   Spawns l85 with Scope, Silencer and AP ammo
; wpn_l85        Spawn l85 with default 0 ammo type and 0 addon flags
; wpn_lr300:3:1:100:200 Spawns lr300 with GL and between 100 to 200 ammo
; ammo_mix and ammo_max are optional and override ammo_* variable for individual weapons
;Note: If you decide to use extra params you must use all 3 params (ie. section:flags:ammo_type)

; Addon Flag ( 0 - no attachment, Scope +1 to flag, GL add +2 to flag, Silencer add + 4 to flag)

; Ammo type corresponds to the ammo_class listed for the weapon. Either 0 or 1. If the first list ammo_class
; Is AP ammo then ammo_type 0 will Set the ammo_type of the weapon to AP ammo and spawn AP ammo
; depending on the box count provided.

; Specify "none" if you want randomly no weapon in that slot
