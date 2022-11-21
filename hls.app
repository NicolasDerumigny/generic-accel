<project xmlns="com.autoesl.autopilot.project" name="accel-corr" top="generic_accel">
    <includePaths/>
    <libraryPaths/>
    <Simulation>
        <SimFlow name="csim" csimMode="0" lastCsimMode="0"/>
    </Simulation>
    <files xmlns="">
        <file name="../test/main.cpp" sc="0" tb="1" cflags=" -Wno-unknown-pragmas" csimflags=" -Wno-unknown-pragmas" blackbox="false"/>
        <file name="accel-corr/src/dma.hpp" sc="0" tb="false" cflags="" csimflags="" blackbox="false"/>
        <file name="accel-corr/src/core.hpp" sc="0" tb="false" cflags="" csimflags="" blackbox="false"/>
        <file name="accel-corr/src/core.cpp" sc="0" tb="false" cflags="" csimflags="" blackbox="false"/>
    </files>
    <solutions xmlns="">
        <solution name="zcu104" status="active"/>
    </solutions>
</project>

