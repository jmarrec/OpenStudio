include OpenStudio::Model

m = Model.new
s1 = Space.new(m)
z1 = ThermalZone.new(m)
s1.setThermalZone(z1)

d = DaylightingControl.new(m)
# d doesn't have a space
raise if !z1.setPrimaryDaylightingControl(d)

s2 = Space.new(m)
z2 = ThermalZone.new(m)
s2.setThermalZone(z2)

ft = OpenStudio::EnergyPlus::ForwardTranslator.new
w = ft.translateModel(m)
puts "===== NUMBER 1 ====="
puts "Now we have a DaylightingControls that has no space. But Zone 1 is refererencing it as its primary daylighting control. And no one complains."
puts w.getObjectsByType("Daylighting:Controls")
puts w.getObjectsByType("Daylighting:ReferencePoint")


puts "===== NUMBER 2 ====="
puts "Now we have a DaylightingControls that in Space 2/ Zone 2. But Zone 1 is refererencing it as its primary daylighting control. And no one complains"
d.setSpace(s2)
w = ft.translateModel(m)
puts w.getObjectsByType("Daylighting:Controls")
puts w.getObjectsByType("Daylighting:ReferencePoint")
