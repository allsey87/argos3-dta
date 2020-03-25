luabt = require('luabt')

function init()	   
   --[[ table of fixed robot parameters ]]--
   robot.constants = {
		m = 20,
		m2 = 100,
		opt_dens = 0.1,
   }
   
   --[[ table of variable robot parameters ]]--
   robot.variables = {
		--[[ create an entry in the robot table to store
			the readings from the ground sensors ]]--
		accumulator = 0.0,
		
		--[[ create an entry in the robot table to store
			the robot's estimates ]]--
		estimate = 0.0,
		prev_iter_estimate = robot.estimate,
		deviation = 0.0,
	   
		--[[ create an entry in the robot table to store
			the robot's state ]]--
		state = "initial_estimation",
		
		--[[ create entries in the robot table to store
			 the robot's time counters ]]--
		timer = 0,
		curr_expl_time = 0,
   
		tot_expl_time = robot.random.exponential(robot.constants.m2),
		tot_diss_time = robot.random.exponential(robot.constants.m),
   }
   
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
        the decision-making process ]]--
   robot.listen = luabt.create({
	   type = "sequence",
	   children = {
		  -- action leaf, update the current robot accumulator info based on its neighbors
		  function()
		  -- local variables to calculate the average over the neighborhood
			local average_est, average_dev, num_neighbors = 0.0, 0.0, 0.0
			for index, message in ipairs(robot.wifi.rx_data) do
			  average_est = average_est + message.est
			  average_dev = average_dev + message.dev
			  num_neighbors = num_neighbors + 1.0
			end
			if num_neighbors > 0 then
				average_est = average_est/num_neighbors 
				average_dev = average_dev/num_neighbors 
				robot.variables.estimate = robot.variables.estimate + 0.5 * (average_est - robot.variables.estimate)
				robot.variables.deviation = robot.variables.deviation + 0.5 * (average_dev - robot.variables.deviation)
			end
		  return true end,
		  }
	   })
   
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
      if reading < 0.075 then
         obstacle_detected = true
      end
   end
   -- tick obstacle avoidance behavior tree
   robot.behavior()
   
	-- sets the current robot task, i.e. foraging, exploring or initial_exploration
	robot.debug.set_task(robot.variables.state) 
      
   -- update the timer
   robot.variables.timer = robot.variables.timer + 1
    
   --~ -- put the robot id and the value of the ground accumlator in a table
   if robot.variables.state == "exploring" then
   --~ the robot is exploring the arena/cache in search for shaded tiles/building blocks
	   --~ decrement the number of time steps a robot is exploring
	   robot.variables.curr_expl_time = robot.variables.curr_expl_time + 1.0
	   --~ count the number of time steps a robot is on a shaded tile
	   if robot.ground.center.reading < 0.75 then
		  robot.variables.accumulator = robot.variables.accumulator + 1.0
	   end
      
      --~ the robot averages its accumulator measurements over robot.variables.tot_diss_time+1 seconds
      if robot.variables.timer >= robot.variables.tot_diss_time+1 then
		  -- update the robot's extimate and deviation from the desired density opt_dens
		  robot.variables.estimate = (robot.variables.estimate * (robot.variables.curr_expl_time - 1) + robot.variables.accumulator) / (1.0*robot.variables.curr_expl_time)
		  robot.variables.deviation = (robot.variables.estimate - robot.constants.opt_dens)
		  robot.variables.prev_iter_estimate = robot.variables.estimate
		  
		  robot.variables.timer = 0
		  robot.variables.curr_expl_time = 1
		  robot.variables.accumulator = 0
		  robot.variables.tot_diss_time = robot.random.exponential(robot.constants.m)
		  
		  -- switch probabilistically to foraging, iff (robot.variables.estimate - robot.constants.opt_dens) < 0
		  local switch_prob = robot.variables.accumulator / robot.variables.curr_expl_time
		  
		  if robot.random.uniform() < robot.constants.opt_dens and robot.variables.deviation < 0 then
			 robot.variables.state = "foraging"
		  end
	  end
	  -- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	   
	  --~ put the robot id and the value of the ground accumlator in a table
      local data = {
         id = robot.id,
         est = robot.variables.estimate,
         dev = robot.variables.deviation
      }
      --~ broadcast that table over wifi
      robot.wifi.tx_data(data)
      
      --~ now listen to others
      robot.listen()
      
   --~ perform the initial_exploration if you just arrived from foraging
   elseif robot.variables.state == "initial_estimation" then
	   --~ decrement the number of time steps a robot is perorming initial_estimation
	   robot.variables.curr_expl_time = robot.variables.curr_expl_time + 1.0
	   
	   --~ count the number of time steps a robot is on a shaded tile
	   if robot.ground.center.reading < 0.75 then
		  robot.variables.accumulator = robot.variables.accumulator + 1.0
	   end
      
      --~ Has the robot finished initial exploration?
      if robot.variables.curr_expl_time >= robot.variables.tot_expl_time then
		  
		  --~ Prepare the robot to switch to exploring
		  robot.variables.state = "exploring"
		  
		  robot.variables.estimate = robot.variables.accumulator / (1.0*robot.variables.curr_expl_time)
		  robot.variables.prev_iter_estimate = robot.variables.estimate
		  robot.variables.accumulator = 0
		  
		  robot.variables.timer = 0
		  robot.variables.curr_expl_time = 1
		  robot.variables.tot_diss_time = robot.random.exponential(robot.constants.m)
	  end
	  -- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   end
   
   -- send the current estimate to the loop functions
   local estimate = string.format("%3.5f", robot.variables.estimate)
   estimate = estimate:gsub(",", ".")
   robot.debug.set_estimate( estimate )
   local gradient = string.format("%3.5f", robot.variables.deviation)
   gradient = gradient:gsub(",", ".")
   robot.debug.set_gradient( gradient )
end

function reset() end
function destroy() end
