luabt = require('luabt')

-- TODO:
-- - To reduce gradient, increase exploration time
-- - Merge the dissemination and the exploration phases; or at least allow them to communicate during the exploration phase

function init()
   --[[ create an entry in the robot table to store
        the readings from the ground sensors ]]--
   robot.accumulator = 0.0
   robot.timer = 0
   --[[ create an entry in the robot table to store
        the robot's estimate ]]--
   robot.estimate = 0.0
   robot.prev_iter_estimate = robot.estimate
   robot.gradient = 0.0
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
			  average_dev = average_dev + message.dev
			  num_neighbors = num_neighbors + 1.0
			end
			if num_neighbors > 0 then
				average_est = average_est/num_neighbors 
				average_dev = average_dev/num_neighbors 
				robot.estimate = robot.estimate + 0.5 * (average_est - robot.estimate)
				robot.gradient = robot.gradient + 0.5 * (average_dev - robot.gradient)
			end
		  return true end,
		  }
	   })
   --[[ create an entry in the robot table to store
        the robot's current state ]]--
   robot.state = "initial_estimation"
   robot.m = 20
   robot.m2 = 100
   robot.expl_modulation = 1
   robot.tot_expl_time = robot.random.exponential(robot.m2)
   robot.curr_expl_time = 0
   robot.tot_diss_time = robot.random.exponential(robot.m)
   robot.curr_diss_time = 0
   
   robot.grad_norm = 1
   robot.opt_dens = 0.1
end

function step()

	robot.debug.set_task(robot.state)
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
      
   robot.timer = robot.timer + 1 
	  
   
   -- tick obstacle avoidance behavior tree
   robot.behavior()   
   
   -- tell the loop functions we want to switch to foraging
   if robot.ground.center.reading < 0.75 then
      if robot.accumulator > 25 and robot.random.uniform(0,1) > 0.25 then
         --~ robot.debug.set_task("foraging");
      end
   end
   --~ -- put the robot id and the value of the ground accumlator in a table
   if robot.state == "exploring" then
       --~ log(string.format("bot %s is exploring", robot.id))
	   --~ -- decrement the number of time steps a robot is exploring
	   robot.curr_expl_time = robot.curr_expl_time + 1.0
	   --~ -- count the number of time steps a robot is on a shaded tile
	   robot.accumulator = 0
	   if robot.ground.center.reading < 0.75 then
		  robot.accumulator = robot.accumulator + 1.0
	   end
      
      --~ THIS IS NECESSARY TO MERGE EXPLORATION AND DISSEMINATION:
      if robot.timer >= robot.tot_diss_time+1 then
		  robot.estimate = (robot.estimate * (robot.curr_expl_time - 1) + robot.accumulator) / (1.0*robot.curr_expl_time)
		  robot.gradient = (robot.estimate - robot.opt_dens)
		  --~ robot.gradient = (robot.gradient * (robot.curr_expl_time - 1) + (robot.prev_iter_estimate - robot.estimate)) / (1.0*robot.curr_expl_time) 
		  --~ robot.gradient = (robot.gradient * (robot.curr_expl_time - 1) + (robot.estimate - robot.opt_dens)) / (1.0*robot.curr_expl_time) 
		  robot.prev_iter_estimate = robot.estimate
		  
		  robot.timer = 0
		  robot.curr_expl_time = 1
		  robot.accumulator = 0
		  robot.tot_diss_time = robot.random.exponential(robot.m)
		  
		  local switch_prob = robot.accumulator / robot.curr_expl_time
		  
		  --~ if robot.estimate < robot.random.uniform()*robot.random.uniform()*robot.random.uniform() and robot.gradient < 0 then
		  if robot.random.uniform() < robot.opt_dens and robot.gradient < 0 then
		  --~ if robot.gradient < 0 then
		  --~ if switch_prob < robot.random.uniform() and robot.gradient < 0 then
			 --~ robot.debug.set_task("foraging")
			 robot.state = "foraging"
			 --~ log(string.format("bot est %.3f", robot.estimate))
		  end
	  end
	  -- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	   
      local data = {
         id = robot.id,
         est = robot.estimate,
         dev = robot.gradient
      }
      --~ -- send that table over wifi
      robot.wifi.tx_data(data)
      --~ log(string.format("bot estimate %s", data.est))
      --~ -- now listen to others
      --~ -- = tick decision-making tree
      robot.decide()
   --~ -- put the robot id and the value of the ground accumlator in a table
   elseif robot.state == "initial_estimation" then
       --~ log(string.format("bot %s is exploring", robot.id))
	   --~ -- decrement the number of time steps a robot is exploring
	   robot.curr_expl_time = robot.curr_expl_time + 1.0
	   --~ -- count the number of time steps a robot is on a shaded tile
	   if robot.ground.center.reading < 0.75 then
		  robot.accumulator = robot.accumulator + 1.0
	   end
      
      --~ THIS IS NECESSARY TO MERGE EXPLORATION AND DISSEMINATION:
      if robot.curr_expl_time >= robot.tot_expl_time then
		  robot.estimate = robot.accumulator / (1.0*robot.curr_expl_time)
		  robot.prev_iter_estimate = robot.estimate
		  
		  robot.state = "exploring"
		  
		  robot.timer = 0
		  robot.curr_expl_time = 1
		  robot.accumulator = 0
		  robot.tot_diss_time = robot.random.exponential(robot.m)
		  robot.grad_norm = math.abs(robot.prev_iter_estimate - 0.5);
	  end
	  -- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	     
   else
      --~ log(string.format("robot %s is confused", robot.id))
   end
   -- send the current estimate to the loop functions
   local estimate = string.format("%3.5f", robot.estimate)
   estimate = estimate:gsub(",", ".")
   robot.debug.set_estimate( estimate )
   local gradient = string.format("%3.5f", robot.gradient)
   gradient = gradient:gsub(",", ".")
   robot.debug.set_gradient( gradient )
   --~ -- print the received data from other robots
   --~ log(string.format("[%s received]", robot.id))
   for index, message in ipairs(robot.wifi.rx_data) do
      --~ log(string.format("%d: %s => %d", index, message.id, message.est))
   end
end

function reset() end
function destroy() end
