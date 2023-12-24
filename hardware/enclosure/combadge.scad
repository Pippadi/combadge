$wallThickness = 1;
$wiggleRoom = 0.5;
$chamberDepth = 10;

$pcbMajorAxis = 45;
$pcbMinorAxis = 31;

$chevronHeight = 13;

$fn = 64;

$cavityLength = $pcbMajorAxis + 2*$wiggleRoom;
$cavityWidth = $pcbMinorAxis + 2*$wiggleRoom;
$baseLength = $cavityLength + 2*$wallThickness;
$baseWidth = $cavityWidth + 2*$wallThickness;

module chevron() {
    linear_extrude($chevronHeight) import("chevron.svg");
}

module annulus(outer_radius, width, height) {
    linear_extrude(height) difference() {
        circle(outer_radius);
        circle(outer_radius - width);
    }
}

module base_top() {
    scale([1, 1, 5/$baseWidth])
    rotate_extrude() 
    intersection() {
       circle($baseWidth/2);
       square([$baseWidth, $baseWidth], center=false);
    }
    annulus($baseWidth/2, 1, 1.75);
}

module base() {
    translate([$baseLength/2, $baseWidth/2, 0]) scale([$baseLength/$baseWidth, 1, 1])
    union() {
        difference() {
            cylinder($chamberDepth, d = $baseWidth);
            cylinder($chamberDepth, d = $cavityWidth);
        }
        translate([0, 0, $chamberDepth]) base_top();
    }
}

module combadge() {
    union() {
        translate([6, -2, 0]) scale([0.85, 0.85, 1]) chevron();            
        base();
    }
}

combadge();