stdA = 44.4;
stdB = 19;
stdC = 8; // dépassement depuis extérieur boitier, non ISO = 27mm, ISO=16.5
stdD = 45;
stdE = 6.1;
stdF = 24;
stdR = 35;
stdW = 33.1; // std would be 35.6mm!!! Legrand (c) and others
stdTh = 2;
camD = 10;
camOffcY = 4 + camD/2;
flashD = 5.5;
flashOffcX = flashD/2+1.5;
flashOffcY = 29;
fixHoleD = 4;
fixHoled = 2;

$fn=20;

basepts = [
    [0,0], [stdA,0], [stdA,stdB], [stdA+stdC, stdB], [stdA+stdC, stdB+stdD],
    [stdA, stdB+stdD], [stdA, stdB*2+stdD], [0, stdB*2+stdD],
    [-stdE, stdB*2+stdD], [-stdE, stdB*2+stdD-stdF], [0, stdB*2+stdD-stdF],
    [0, stdF], [-stdE, stdF], [-stdE, 0]
];


difference() {
    union() {
        baseISO();
        mountingESP();
        mountingHoles();
    }
    screwHoles();
    camHole();
    ledStatusHole();
    flashHole();
    wiresHole();
}

mirror([1,0,0]) {
    translate([10, 0, 0]) {
    difference() {
        union() {
            baseISO();
            mountingESP();
            mountingHolesB();
        }
        camHole();
        ledStatusHole();
        //flashHole();
        wiresHole();
    }
    }
}

module camHole() {
    color("red")
    translate([stdE+stdA+stdC-stdTh-stdTh/2, stdB+stdD-camOffcY-stdTh, stdW/2])
    rotate([0,90,0])
    cylinder(r=camD/2,h=stdTh*2);
}

module flashHole() {
    color("yellow")
    translate([stdE+stdA+stdC-stdTh-stdTh/2, stdB+stdD-flashOffcY-stdTh, stdTh+flashOffcX])
    rotate([0,90,0])
    cylinder(r=flashD/2,h=stdTh*2);
}

module ledStatusHole() {
    color("red")
    translate([stdE+stdA+stdC-stdTh-stdTh/2, stdB+stdD-flashOffcY-10, stdTh+8])
    rotate([0,90,0]) {
        cylinder(r=3/2,h=stdTh*2);
    }
}

module mountingESP() {
    translate([stdE+stdA+stdC-stdTh-2.5-8, stdB+stdD-stdTh-8-24])
    linear_extrude(height = stdW/2)
    square([2.5,8]);
    
    translate([stdE+stdA+stdC-stdTh-2.5-8-11.5, stdB+stdD-stdTh-20])
    linear_extrude(height = stdW/2)
    square([2.5,20]);
}

module wiresHole() {
    color("black") {
        translate([15, stdB+stdD+stdB+stdTh/2, stdW/2])
        rotate([90,90,0])
        cylinder(r=15/2,h=stdTh*2);

        translate([35, stdB+stdD+stdB+stdTh/2, stdW/2])
        rotate([90,90,0])
        cylinder(r=15/2,h=stdTh*2);
        
        translate([15, stdTh*1.5, stdW/2])
        rotate([90,90,0])
        cylinder(r=15/2,h=stdTh*2);

        translate([35, stdTh*1.5, stdW/2])
        rotate([90,90,0])
        cylinder(r=15/2,h=stdTh*2);
    }
}

module mountingHoles() {
    translate([stdTh*1.5, stdTh*1.5, 0])
    rotate([0,0,0])
    cylinder(r=fixHoleD/2,h=stdW/2);

    translate([stdTh*1.5+stdA, stdTh*1.5, 0])
    rotate([0,0,0])
    cylinder(r=fixHoleD/2,h=stdW/2);
    
    translate([stdTh*1.5, -stdTh*1.5+stdB*2+stdD, 0])
    rotate([0,0,0])
    cylinder(r=fixHoleD/2,h=stdW/2);

    translate([stdTh*1.5+stdA, -stdTh*1.5+stdB*2+stdD, 0])
    rotate([0,0,0])
    cylinder(r=fixHoleD/2,h=stdW/2);

    translate([stdTh/2, stdB*2+stdD-stdF-3])
    linear_extrude(height = stdW/2)
    rotate([0,0,-15])
    square([2.5,4]);

    translate([stdTh*1.5, stdF, 0])
    cylinder(r=fixHoled/2,h=stdW/2);
}

module screwHoles() {
    translate([stdTh*1.5, stdTh*1.5, 0])
    rotate([0,0,0])
    cylinder(r=fixHoled/1.7,h=stdW/2);

    translate([stdTh*1.5+stdA, stdTh*1.5, 0])
    rotate([0,0,0])
    cylinder(r=fixHoled/1.7,h=stdW/2);
    
    translate([stdTh*1.5, -stdTh*1.5+stdB*2+stdD, 0])
    rotate([0,0,0])
    cylinder(r=fixHoled/1.7,h=stdW/2);

    translate([stdTh*1.5+stdA, -stdTh*1.5+stdB*2+stdD, 0])
    rotate([0,0,0])
    cylinder(r=fixHoled/1.7,h=stdW/2);
}

module mountingHolesB() {
    union() {
    union() {
        translate([stdTh*1.5, stdTh*1.5, 0])
        rotate([0,0,0])
        cylinder(r=fixHoleD/2,h=stdW/2);

        translate([stdTh*1.5+stdA, stdTh*1.5, 0])
        rotate([0,0,0])
        cylinder(r=fixHoleD/2,h=stdW/2);
        
        translate([stdTh*1.5, -stdTh*1.5+stdB*2+stdD, 0])
        rotate([0,0,0])
        cylinder(r=fixHoleD/2,h=stdW/2);

        translate([stdTh*1.5+stdA, -stdTh*1.5+stdB*2+stdD, 0])
        rotate([0,0,0])
        cylinder(r=fixHoleD/2,h=stdW/2);
    }
    union() {
        translate([stdTh*1.5, stdTh*1.5, 0])
        rotate([0,0,0])
        cylinder(r=fixHoled/2,h=stdW/2+4);

        translate([stdTh*1.5+stdA, stdTh*1.5, 0])
        rotate([0,0,0])
        cylinder(r=fixHoled/2,h=stdW/2+4);
        
        translate([stdTh*1.5, -stdTh*1.5+stdB*2+stdD, 0])
        rotate([0,0,0])
        cylinder(r=fixHoled/2,h=stdW/2+4);

        translate([stdTh*1.5+stdA, -stdTh*1.5+stdB*2+stdD, 0])
        rotate([0,0,0])
        cylinder(r=fixHoled/2,h=stdW/2+4);
    }
}

    translate([stdTh/2, stdB*2+stdD-stdF-3])
    linear_extrude(height = stdW/2)
    rotate([0,0,-15])
    square([2.5,4]);

    translate([stdTh*1.5, stdF, 0])
    cylinder(r=fixHoled/2,h=stdW/2);
}

module baseISO() {
     {
        translate([stdE, 0, 0]) {
            
        color("red") linear_extrude(stdTh)color("red") polygon(points=basepts);
        
        linear_extrude(height = stdW/2) {
          difference() {
         offset(delta = 0) {
          polygon(points=basepts);
         }
         offset(delta = -stdTh) {
           polygon(points=basepts);
         }
        }
       }
     }
 }
}