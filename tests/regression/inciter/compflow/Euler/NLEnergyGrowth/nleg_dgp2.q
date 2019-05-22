# vim: filetype=sh:
# This is a comment
# Keywords are case-sensitive

title "Euler equations computing nonlinear energy growth"

inciter
  nstep 40   # Max number of time steps
  dt   2.5e-4 # Time step size
  ttyi 5      # TTY output interval
  scheme dgp2

  compflow

    depvar u
    physics euler
    problem nl_energy_growth
    alpha 0.25
    betax 1.0
    betay 0.75
    betaz 0.5
    r0 2.0
    ce -1.0
    kappa 0.8

    material
      id 1
      gamma 1.66666666666667 # =5/3 ratio of specific heats
    end

    bc_dirichlet
      sideset 1 2 3 4 5 6 end
    end

  end

  plotvar
    interval 5
  end

  diagnostics
    interval  5
    format    scientific
    error l2
  end

end