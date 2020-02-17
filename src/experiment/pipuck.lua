luabt = require('luabt')

function init()
   --[[ create an entry in the robot table to store
        the readings from the ground sensors ]]--
   robot.accumulator = 0
   --[[ create an entry in the robot table to store
        the obstacle avoidance behavior tree ]]--
   robot.behavior = luabt.create({
      type = "selector",
      children = {{
         type = "sequence",
         children = {
            function() return false, obstacle_detected end,
            function() robot.differential_drive.set_target_velocity(0.05, 0.05) return true end,
         }},
         function() robot.differential_drive.set_target_velocity(0.05, -0.05) return true end,
      }
   })
   --[[ create an entry in the robot table to store
        the robot's current state ]]--
   robot.state = "disseminating"
end

function step()
   -- process obstacles
   obstacle_detected = false
   local rangefinders = {
      far_left  = robot.rangefinders[7].reading,
      left      = robot.rangefinders[8].reading,
      right     = robot.rangefinders[1].reading,
      far_right = robot.rangefinders[2].reading,
   }
   for rangefinder, reading in pairs(rangefinders) do
      if reading < 0.1 then
         obstacle_detected = true
      end
   end
   -- tick obstacle avoidance behavior tree
   robot.behavior()
   -- tell the loop functions we want to switch to foraging
   if robot.ground.center.reading < 0.75 then
      robot.accumulator = robot.accumulator + 1
      if robot.accumulator > 25 and robot.random.uniform(0,1) > 0.25 then
         robot.debug.loop_functions("foraging");
      end
   end
   -- put the robot id and the value of the ground accumlator in a table
   if robot.state == "disseminating" then
      local data = {
         id = robot.id,
         acc = robot.accumulator
      }
      -- send that table over wifi
      robot.wifi.tx_data(data)
   end
   -- print the received data from other robots
   log(string.format("[%s received]", robot.id))
   for index, message in ipairs(robot.wifi.rx_data) do
      log(string.format("%d: %s => %d", index, message.id, message.acc))
   end
end

function reset() end
function destroy() end
