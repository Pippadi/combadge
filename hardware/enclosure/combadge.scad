$wallThickness = 1;
$wiggleRoom = 0.5;
$chamberDepth = 10;

$pcbMajorAxis = 45;
$pcbMinorAxis = 31;

$chevronHeight = 13;

$fn = 128;

$chamberLength = $pcbMajorAxis + 2*$wiggleRoom;
$chamberWidth = $pcbMinorAxis + 2*$wiggleRoom;
$baseLength = $chamberLength + 2*$wallThickness;
$baseWidth = $chamberWidth + 2*$wallThickness;

module scale_width() {
    translate([$baseLength/2, $baseWidth/2, 0])
        scale([$baseLength/$baseWidth, 1, 1])
        children();
}

module chevron() {
    difference() {
        linear_extrude($chevronHeight) import("chevron.svg");
        translate([10, 40, 0]) cube([30, 30, 8]);
    }
}

module annulus(outer_radius, width, height) {
    linear_extrude(height) difference() {
        circle(outer_radius);
        circle(outer_radius - width);
    }
}

module base_top() {
    scale([1, 1, 5.5/$baseWidth])
        rotate_extrude() 
        intersection() {
           circle($baseWidth/2);
           square([$baseWidth, $baseWidth], center=false);
        }
    annulus($baseWidth/2, 1, 1.75);
}

module base() {
    scale_width()
    union() {
        cylinder($chamberDepth, d = $baseWidth);
        translate([0, 0, $chamberDepth]) base_top();
    }
}

module combadge() {
    difference() {
        union() {
            color("#c0c0c0") translate([6, -2, 0]) scale([0.85, 0.85, 1]) chevron();
            color("#ffb700") base();
        }
        scale_width() cylinder($chamberDepth, d = $chamberWidth);
    }
}

combadge();