luabt = require('luabt')

-- TODO:
-- - To reduce deviation, increase exploration time
-- - Merge the dissemination and the exploration phases; or at least allow them to communicate during the exploration phase

function init()
   --[[ create an entry in the robot table to store
        the readings from the ground sensors ]]--
   robot.accumulator = 0.0
   --[[ create an entry in the robot table to store
        the robot's estimate ]]--
   robot.estimate = 0.5
   robot.prev_iter_estimate = robot.estimate
   robot.deviation = 0.0
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
   robot.decide = luabt.create({
	   type = "sequence",
	   children = {
		  -- action leaf, update the current robot accumulator info based on its neighbors
		  function()
			local average_est, average_dev, num_neighbors = 0.0, 0.0, 0.0
			for index, message in ipairs(robot.wifi.rx_data) do
			  average_est = average_est + message.est
			  average_dev = average_dev + message.est
			  num_neighbors = num_neighbors + 1.0
			end
			if num_neighbors > 0 then
				average_est = average_est/num_neighbors 
				average_dev = average_dev/num_neighbors 
				robot.estimate = robot.estimate + 0.5 * (average_est - robot.estimate)
				--~ robot.deviation = robot.deviation + 0.5 * (math.abs(robot.prev_iter_estimate - robot.estimate) - robot.deviation)
				--~ robot.deviation = robot.deviation + 0.5 * (average_dev - robot.deviation)
				--~ robot.prev_iter_estimate = robot.estimate
			end
		  return true end,
		  }
	   })
   --[[ create an entry in the robot table to store
        the robot's current state ]]--
   robot.state = "exploring"
   robot.m = 50
   robot.expl_modulation = 1
   robot.tot_expl_time = robot.random.exponential(10*robot.m*robot.expl_modulation)
   robot.curr_expl_time = 0
   robot.tot_diss_time = robot.random.exponential(robot.m)
   robot.curr_diss_time = 0
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
   --~ log(string.format("robot state %s", robot.state))
   --~ log(string.format("robot curr_expl_time %d", robot.curr_expl_time))
   --~ log(string.format("robot curr_diss_time %d", robot.curr_diss_time))
   -- tell the loop functions we want to switch to foraging
   --~ if robot.ground.center.reading < 0.75 then
	  --~ if robot.accumulator > 25 and robot.random.uniform(0,1) > 0.25 then
		 --~ robot.debug.loop_functions("foraging");
	  --~ end
   --~ end
   
   
   -- tell the loop functions we want to switch to foraging
   if robot.ground.center.reading < 0.75 then
      if robot.accumulator > 25 and robot.random.uniform(0,1) > 0.25 then
         --~ robot.debug.loop_functions("foraging");
      end
   end
   --~ -- put the robot id and the value of the ground accumlator in a table
   if robot.state == "disseminating" then
      --~ log(string.format("bot %s is disseminating", robot.id))
	   --~ -- decrement the number of time steps a robot is disseminating
	   robot.curr_diss_time = robot.curr_diss_time + 1
	   
      local data = {
         id = robot.id,
         est = robot.estimate,
         dev = robot.deviation
      }
      --~ -- send that table over wifi
      robot.wifi.tx_data(data)
      --~ log(string.format("bot estimate %s", data.est))
      --~ -- now listen to others
      --~ -- = tick decision-making tree
      robot.decide()
      
      if robot.curr_diss_time >= robot.tot_diss_time then
		 robot.state = "exploring"
		 --~ robot.expl_modulation = math.max(1, robot.deviation * 10)
		 robot.expl_modulation = 1
	     robot.tot_expl_time = robot.random.exponential(robot.m*robot.expl_modulation)
	     robot.curr_expl_time = 0
	     robot.prev_iter_estimate = robot.estimate
	     if robot.random.uniform(0,1) > 0.75 then
			--~ robot.debug.loop_functions("foraging");
		end
	  end
   elseif robot.state == "exploring" then
       --~ log(string.format("bot %s is exploring", robot.id))
	   --~ -- decrement the number of time steps a robot is exploring
	   robot.curr_expl_time = robot.curr_expl_time + 1.0
	   --~ -- count the number of time steps a robot is on a shaded tile
	   if robot.ground.center.reading < 0.75 then
		  robot.accumulator = robot.accumulator + 1.0
	   end
      
      if robot.curr_expl_time >= robot.tot_expl_time then
		 robot.state = "disseminating"
		 robot.estimate = robot.accumulator/(1.0*robot.curr_expl_time)
		 robot.accumulator = 0.0
		 robot.curr_diss_time = 0
		 robot.tot_diss_time = robot.random.exponential(robot.m)
		 robot.deviation = math.abs(robot.prev_iter_estimate - robot.estimate)
		 --~ robot.deviation = 0.1
	  end	   
   else
      --~ log(string.format("robot %s is confused", robot.id))
   end
   -- send the current estimate to the loop functions
   local estimate = string.format("%3.5f", robot.estimate)
   estimate = estimate:gsub(",", ".")
   robot.debug.set_estimate( estimate )
   local deviation = string.format("%3.5f", robot.deviation)
   deviation = deviation:gsub(",", ".")
   robot.debug.set_deviation( deviation )
   --~ -- print the received data from other robots
   --~ log(string.format("[%s received]", robot.id))
   for index, message in ipairs(robot.wifi.rx_data) do
      --~ log(string.format("%d: %s => %d", index, message.id, message.est))
   end
end

function reset() end
function destroy() end
