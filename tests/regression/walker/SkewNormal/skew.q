title "Example problem"

walker

  term 10.0 # Max time
  dt 0.001 # Time step size
  npar 1000 # Number of particles
  ttyi 1000 # TTY output interval

  rngs
   r123_philox end
  end

  skew-normal
    depvar m
    init zero
    coeff const_coeff
    ncomp 2
    T 1.0 3.5 end
    sigmasq 0.04 0.25 end
    lambda 100.0 -50.0 end
    rng r123_philox
  end

  statistics
    interval 2
    <m1m1> <m2m2>
  end

end
